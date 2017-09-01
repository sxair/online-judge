#include "lang.h"

#define OJ_JG 1
#define OJ_WA 2
#define OJ_CE 3
#define OJ_MLE 4
#define OJ_OLE 5
#define OJ_TLE 6
#define OJ_PE 7
#define OJ_AC 8
#define OJ_RE 10
#define OJ_SF 11 //Segmentation fault sig:11
#define OJ_FP 12 //Floating Point Exception sig:8
#define OJ_SC 13 //not all SYSTEM CALL

const char status_map[][8] = {"","","wa","ce","MLE","OLE","TLE","PE","AC","","RE"};

char RUN_ROOM[32],prefix[32];
int run_id; // run in which room

/*
* if in oj ,judge_for==0 then status_id ..
* in other, judge_for & other_BASE ->check is what,and judge_for ^ other_BASE is status
* type_id is other_id
*/
unsigned status_id, problem_id, judge_for, true_problem_id;
unsigned type_id;

long name;
int lang, time_limit, memory_limit,time_used, memory_used, spj, judge_cnt, time_limit_second;
int user_id;

void set_status(int status) {
#ifdef DEBUG
    debug("finaly %d\n",status);
#endif // DEBUG
    if(judge_for == 0) {
        execsql("UPDATE `oj_status` SET `status`=%d,`time`=%d,`memory`=%d WHERE id=%u", status, time_used, memory_used, status_id);
        execsql("DELETE FROM `judges` WHERE status_id=%u AND judge_for=%d", status_id, judge_for);

        if(status == OJ_AC) {
            execsql("UPDATE `oj_problems` SET `accepted`= `accepted` + 1 WHERE id=%u", problem_id);
            execsql("UPDATE `oj_problem_infos` SET `ac_user`= `ac_user` + 1 WHERE id=%u AND EXISTS(SELECT * FROM `oj_solved_problems` \
WHERE `user_id`=%u AND `problem_id`=%u AND `accepted` = 0)", problem_id, user_id, problem_id);
            execsql("UPDATE `oj_solved_problems` SET `accepted`= `accepted` + 1 WHERE user_id=%u AND problem_id=%u ", user_id, problem_id);
        } else {
            int zz = status / OJ_TEST_MAX;
            if(zz>10 || zz < 0) zz = 10;
            execsql("UPDATE `oj_problem_infos` SET `%s`= `%s` + 1 WHERE id=%u", status_map[zz], status_map[zz], problem_id);
        }
    } else {
    }
}

void set_ce(const char *s) {
    if (s == NULL) {
        char ce_buf[2048] = "";
        FILE *fp = fopen("ce.txt", "rb");
        if (fp == NULL) {
            execsql("INSERT INTO `oj_ces`(`status_id`, `content`) VALUES (%u,'%s,%s')", status_id, "编译错误信息无法读取", "请与管理员联系");
            set_status(OJ_CE);
            exit(1);
        }
        if(fread(ce_buf, 1, 2047, fp)) {
            execsql("INSERT INTO `oj_ces`(`status_id`, `content`) VALUES (%u, '%s')", status_id, ce_buf);
        }
    } else {
        execsql("INSERT INTO `oj_ces`(`status_id`, `content`) VALUES (%u,'%s,%s')", status_id, s, "请与管理员联系");
    }
    set_status(OJ_CE);
    exit(1);
}

// 语言的后缀
const char LANG[][8] = {"c", "cpp", "java"};
int solve() {
    switch (lang) {
    case 0:
        return c_solve();
    case 1:
        return c_solve();
    }
    set_ce("没有该语言的处理器");
    return OJ_WA;
}
/*
DROP PROCEDURE `oj_judge_start`;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `oj_judge_start`(IN `in_status_id` INT UNSIGNED, OUT `out_lang` TINYINT, OUT `out_pro_id` INT UNSIGNED, OUT `out_user_id` INT UNSIGNED, OUT `out_code` TEXT, OUT `out_true_pro_id` INT UNSIGNED)
    READS SQL DATA
BEGIN
	UPDATE `oj_status` SET `status`=1 WHERE id=in_status_id;
	SELECT `problem_id`,`lang`,`user_id` INTO out_pro_id,out_lang,out_user_id FROM `oj_status` WHERE `id`=in_status_id;
   	SELECT `problem_id` INTO out_true_pro_id FROM `oj_problems` WHERE `id`=out_pro_id;
    SELECT `code` INTO out_code FROM oj_codes where `status_id`=in_status_id;
END$$
DELIMITER ;
*/
void judge_start() {
    if(!connect_mysql()) {
        exit(1);
    }
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[64];
    if (judge_for == 0) {
        execsql("CALL `oj_judge_start`(%u, @p1, @p2, @p3, @p4, @p5)", status_id);
        execsql("SELECT @p1, @p2, @p3, @p4, @p5");
    } else {
    }
    res = mysql_store_result(conn);
    if(res != NULL && (row = mysql_fetch_row(res)) != NULL) {
        lang = strtoul(row[0], 0, 10);
        problem_id = strtoul(row[1], 0, 10);
        user_id = strtoul(row[2], 0, 10);
        write_file(row[3], "./%ld.%s", name, LANG[lang]);
        true_problem_id = strtoul(row[4],0,10);
        mysql_free_result(res);
        res = NULL;
    } else {
        warning("启动失败：%s\n",mysql_error(conn));
        set_ce("启动失败");
    }
#ifdef DEBUG
    debug("lang:%s pro_id:%u user_id:%u\n", LANG[lang], problem_id, user_id);
#endif // DEBUG
}

void get_problem_info() {
    MYSQL_RES *res;
    MYSQL_ROW row;
    execsql("SELECT time_limit,memory_limit,spj,judge_cnt FROM problems WHERE id=%u", true_problem_id);
    res = mysql_store_result(conn);
    if(res != NULL && (row = mysql_fetch_row(res)) != NULL) {
        time_limit = atoi(row[0]);
        // 题目的是毫秒计算
        time_limit_second = (time_limit + 999) / 1000;
        memory_limit = atoi(row[1]);
        spj = atoi(row[2]);
        judge_cnt = atoi(row[3]);

        mysql_free_result(res);
        res = NULL;
    } else {
        warning("获取题目信息错误：%s\n",mysql_error(conn));
        set_ce("获取题目信息错误");
    }
#ifdef DEBUG
    debug("true_problem:%d  time_limit:%d mem_limit:%d spj:%d\n", true_problem_id, time_limit, memory_limit, spj);
#endif // DEBUG
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("judging need judge_id run_id\n");
        exit(1);
    }
    // 为了文件不被include，随便写的随机数做文件名
    name = time(NULL);
    unsigned long long judge_id = strtoull(argv[1],0,10);
    status_id = (unsigned)(judge_id & 0xffffffff);
    judge_for = (unsigned)(judge_id >> 32);
    if(judge_for) {
        if(judge_for & CONTEST_BASE) {
            type_id = status_id;
            status_id = judge_for ^ CONTEST_BASE;
        }
    }
    run_id = atoi(argv[2]);
    sprintf(RUN_ROOM, "%s/run%d", RUN_PATH, run_id);
#ifdef DEBUG
    debug("home: %s judge_id:%llu status_id: %u run_id: %d judge_for:%u RUN_ROOM: %s\n", HOME_PATH, judge_id, status_id, run_id, judge_for, RUN_ROOM);
#endif // DEBUG
    execcmd("rm -f %s/*",RUN_ROOM);
    if(chdir(RUN_ROOM)) {
        warning("chdir to %s failed\n", RUN_ROOM);
        exit(1);
    }
    // 获取各种数据库信息
    judge_start();
    get_problem_info();
    int t = solve();
    t == OJ_CE ? set_ce((char *)NULL) : set_status(t);

    if(conn != NULL) mysql_close(conn);
    conn = NULL;
    return 0;
}

