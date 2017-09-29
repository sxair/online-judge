#include "from.h"

//one contest status no more than 268435456-1
// 0 is oj_problem
#define CONTEST_BASE 0x10000000
#define DIY_BASE     0x20000000

#define TYPE_OJ(judge_for) judge_for==0
#define TYPE_CONTEST(judge_for) judge_for&CONTEST_BASE==CONTEST_BASE
#define TYPE_DIY(judge_for) judge_for&DIY_BASE==DIY_BASE

#define CONTEST_ID(judge_for) judge_for^CONTEST_BASE
#define DIY_ID(judge_for) judge_for^DIY_BASE

unsigned problem_id;
unsigned type_id, user_id;

extern unsigned status_id, judge_for;
extern int time_limit, time_limit_second, memory_limit,time_used, memory_used;
extern int spj, judge_cnt, lang, true_problem_id;

//#define OJ_CI 1 //Compiling
//#define OJ_CE 2
//#define OJ_AC 3
//#define OJ_SE 4 //System Error
//#define OJ_RI 5 //Running
//#define OJ_WA 6
//#define OJ_MLE 7
//#define OJ_OLE 8
//#define OJ_TLE 9
//#define OJ_PE 10
//#define OJ_RE 11
//#define OJ_SF 12 //Segmentation fault sig:11
//#define OJ_FPE 13 //Floating Point Exception sig:8
//#define OJ_NASC 14 //not all SYSTEM CALL

const char status_map[][8] = {"","","ce","ac", "", "","WA","MLE","OLE","TLE","PE","RE"};

// 语言的后缀
const char LANG[][8] = {"", "c", "cpp", "java", "py", "py"};

void set_ce() {
    char ce_buf[MAX_BUFF] = "";
    FILE *fp = fopen("ce.txt", "rb");
    if (fp == NULL) {
        execsql("INSERT INTO `oj_ces`(`status_id`, `content`) VALUES (%u,'%s%s')", status_id, "编译错误信息无法读取,请与管理员联系");
    } else if(fread(ce_buf, 1, MAX_BUFF, fp)) {
        execsql("INSERT INTO `oj_ces`(`status_id`, `content`) VALUES (%u, '%s')", status_id, ce_buf);
    }
}

void set_status(unsigned status) {
#ifdef DEBUG
    debug("status %u\n",status);
#endif // DEBUG
    if(judge_for == 0) {
        if(status == OJ_CI || status / MAX_TEST == OJ_RI) {
            execsql("UPDATE `oj_status` SET `status`=%u WHERE id=%u", status, status_id);
            return ;
        }
        if(status == OJ_CE) set_ce();
        execsql("UPDATE `oj_status` SET `status`=%u,`time`=%d,`memory`=%d WHERE id=%u", status, time_used, memory_used, status_id);
        //judges table used in judge.cpp
        execsql("DELETE FROM `judges` WHERE status_id=%u AND judge_for=%d", status_id, judge_for);
        if(status == OJ_AC) {
            execsql("UPDATE `oj_problems` SET `accepted`= `accepted` + 1 WHERE id=%u", problem_id);
            execsql("UPDATE `user_infos` SET `accepted`= `accepted` + 1 WHERE id=%u", user_id);
            execsql("UPDATE `oj_problem_infos` SET `ac_user`= `ac_user` + 1 WHERE id=%u AND EXISTS(SELECT * FROM `oj_solved_problems` \
WHERE `user_id`=%u AND `problem_id`=%u AND `accepted` = 0)", problem_id, user_id, problem_id);
            execsql("UPDATE `oj_solved_problems` SET `accepted`= `accepted` + 1 WHERE user_id=%u AND problem_id=%u ", user_id, problem_id);
        } else if(status != OJ_SE){
            int zz;
            if(status != OJ_CE){
                zz =  status / MAX_TEST;
            }
            if(zz > OJ_RE || zz < 0) zz = OJ_RE;
            execsql("UPDATE `oj_problem_infos` SET `%s`= `%s` + 1 WHERE id=%u", status_map[zz], status_map[zz], problem_id);
        }
    } else {
    }
}

int get_problem_info() {
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
        set_status(OJ_SE);
        return OJ_SE;
    }
#ifdef DEBUG
    debug("true_problem:%d  time_limit:%d mem_limit:%d judge_cnt:%d spj:%d\n", true_problem_id, time_limit, memory_limit, judge_cnt, spj);
#endif // DEBUG
    return 0;
}

int judge_start() {
#ifdef DEBUG
    if(TYPE_OJ(judge_for)) {
        debug("judge_id:%llu status_id: %u type:oj RUN_ROOM: %s\n", judge_id, status_id, RUN_ROOM);
    } else if(TYPE_CONTEST(judge_for)) {
        debug("judge_id:%llu status_id: %u type:contest con_id:%d RUN_ROOM: %s\n", judge_id, status_id, CONTEST_ID(judge_for),RUN_ROOM);
    } else if(TYPE_DIY(judge_for)) {
        debug("judge_id:%llu status_id: %u type:diy diy_id:%d RUN_ROOM: %s\n", judge_id, status_id, DIY_ID(judge_for),RUN_ROOM);
    }
#endif // DEBUG
    if(!connect_mysql()) {
        exit(OJ_SE);
    }
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[64];
    if (TYPE_OJ(judge_for)) {
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
        execsql("CALL `oj_judge_start`(%u, @p1, @p2, @p3, @p4, @p5)", status_id);
        execsql("SELECT @p1, @p2, @p3, @p4, @p5");
    } else if(TYPE_CONTEST(judge_for)) {
    } else if(TYPE_DIY(judge_for)) {
    } else {
        exit(OJ_SE);
    }
    res = mysql_store_result(conn);
    if(res != NULL && (row = mysql_fetch_row(res)) != NULL) {
        lang = strtoul(row[0], 0, 10);
        problem_id = strtoul(row[1], 0, 10);
        user_id = strtoul(row[2], 0, 10);
        if(lang == LANG_JAVA) {
            write_file(row[3], "./Main.%s", LANG[lang]);
        } else {
            write_file(row[3], "./%ld.%s", name, LANG[lang]);
        }
        true_problem_id = strtoul(row[4],0,10);
        mysql_free_result(res);
        res = NULL;
    } else {
        warning("启动失败：%s\n",mysql_error(conn));
        return OJ_SE;
    }
#ifdef DEBUG
    debug("lang:%s pro_id:%u user_id:%u\n", LANG[lang], problem_id, user_id);
#endif // DEBUG
    return get_problem_info();
}
