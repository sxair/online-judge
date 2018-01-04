#include "soj-inc/provider.h"

unsigned problem_id, user_id;


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
        execsql("INSERT INTO `oj_ces`(`status_id`, `content`) VALUES (%u,'%s')", status_id, "编译错误信息无法读取,请与管理员联系");
    } else if(fread(ce_buf, 1, MAX_BUFF, fp)) {
        char sql[MAX_BUFF << 1] = "INSERT INTO `oj_ces`(`content`, `status_id`) VALUES ('", *s;
        s = sql + strlen(sql);
        mysql_real_escape_string(conn, s, ce_buf,strlen(ce_buf));
        execsql("%s',%u)", sql, status_id);
    }
}

void set_oc_status(int c) {
    char ce_buf[MAX_BUFF] = "", sql[MAX_BUFF << 1] = "UPDATE `admin_status` SET `ce` = '", *s;
    FILE *fp = fopen("ce.txt", "rb");
    if(fp != NULL && fread(ce_buf, 1, MAX_BUFF, fp));
    int status = OJ_AC;
    if(c) {
        status = OJ_CE;
    } else {
        execcmd("cp ./Main %s/%u/Main", DATA_PATH, status_id);
    }
    s = sql + strlen("UPDATE `admin_status` SET `ce` = '");
    mysql_real_escape_string(conn, s, ce_buf,strlen(ce_buf));
    execsql("%s',`status` = %d WHERE id=%d", sql, status, status_id);
}

void set_status(unsigned status) {
#ifdef DEBUG
    printf("status %u\n",status);
#endif // DEBUG
    if(judge_for == 0) {
        if(status == OJ_CI || status / MAX_TEST == OJ_RI) {
            execsql("UPDATE `oj_status` SET `status`=%u WHERE id=%u", status, status_id);
            return ;
        }
        if(status == OJ_CE) set_ce();
        execsql("UPDATE `oj_status` SET `status`=%u,`time`=%u,`memory`=%u WHERE id=%u", status, time_used, memory_used, status_id);
        if(status == OJ_AC) {
            execsql("UPDATE `oj_problems` SET `accepted`= `accepted` + 1 WHERE id=%u", problem_id);
            execsql("UPDATE `user_infos` SET `accepted`= `accepted` + 1 WHERE id=%u", user_id);
            execsql("UPDATE `oj_problem_infos` SET `ac_user`= `ac_user` + 1 WHERE id=%u AND EXISTS(SELECT * FROM `oj_solved_problems` \
WHERE `user_id`=%u AND `problem_id`=%u AND `accepted` = 0)", problem_id, user_id, problem_id);
            execsql("UPDATE `oj_solved_problems` SET `accepted`= `accepted` + 1 WHERE user_id=%u AND problem_id=%u ", user_id, problem_id);
        } else if(status != OJ_SE) {
            int zz;
            if(status != OJ_CE) {
                zz =  status / MAX_TEST;
            }
            if(zz > OJ_RE || zz < 0) zz = OJ_RE;
            execsql("UPDATE `oj_problem_infos` SET `%s`= `%s` + 1 WHERE id=%u", status_map[zz], status_map[zz], problem_id);
        }
    } else if(TYPE_ADMIN(judge_for)){
        if(status == OJ_CE) {
            set_oc_status(OJ_CE);
        } else {
            execsql("UPDATE `admin_status` SET `status` = %d, `time` = %u, `memory`=%u WHERE id=%d", status, time_used, memory_used, status_id);
        }
    }
}

void delete_judge() {
    execsql("delete from `judges` where status_id = %d and judge_for = %d",status_id,judge_for);
}

int get_problem_info() {
    MYSQL_RES *res;
    MYSQL_ROW row;
    execsql("SELECT time_limit,memory_limit,spj,judge_cnt FROM problems WHERE id=%u", true_problem_id);
    if((res = mysql_store_result(conn)) != NULL && (row = mysql_fetch_row(res)) != NULL) {
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
    printf("true_problem:%d  time_limit:%d mem_limit:%d judge_cnt:%d spj:%d\n", true_problem_id, time_limit, memory_limit, judge_cnt, spj);
#endif // DEBUG
    return 0;
}

int judge_start() {
#ifdef DEBUG
    if(TYPE_OJ(judge_for)) {
        printf("status_id: %u type:oj RUN_ROOM: %s\n", status_id, RUN_ROOM);
    } else if(TYPE_CONTEST(judge_for)) {
        printf("status_id: %u type:contest con_id:%d RUN_ROOM: %s\n", CONTEST_STATUS_ID(judge_for), status_id, RUN_ROOM);
    } else if(TYPE_DIY(judge_for)) {
        printf("status_id: %u type:diy diy_id:%d RUN_ROOM: %s\n", DIY_STATUS_ID(judge_for), status_id, RUN_ROOM);
    } else if(TYPE_ADMIN(judge_for)) {
        printf("status_id: %u type:admin RUN_ROOM: %s\n", status_id, RUN_ROOM);
    } else if(TYPE_OC(judge_for)) {
        printf("status_id: %u type:only compile RUN_ROOM: %s\n", status_id, RUN_ROOM);
    }
#endif // DEBUG
    if(conn == NULL && !connect_mysql()) {
        exit(OJ_SE);
    }
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[64];
    if (TYPE_OJ(judge_for)) {
        execsql("CALL `oj_judge_start`(%u, @p1, @p2, @p3, @p4, @p5)", status_id);
        execsql("SELECT @p1, @p2, @p3, @p4, @p5");
    } else if(TYPE_CONTEST(judge_for)) {
    } else if(TYPE_DIY(judge_for)) {
    } else if(TYPE_ADMIN(judge_for) || TYPE_OC(judge_for)) {
        execsql("SELECT `lang`,`problem_id`,`user_id`,`code` FROM `admin_status` WHERE `id` = %d", status_id);
    } else {
        exit(OJ_SE);
    }
    if((res = mysql_store_result(conn)) != NULL && (row = mysql_fetch_row(res)) != NULL) {
        lang = strtoul(row[0], 0, 10);
        problem_id = strtoul(row[1], 0, 10);
        user_id = strtoul(row[2], 0, 10);
        write_file(row[3], "./Main.%s", LANG[lang]); // store code
        if(!TYPE_ADMIN(judge_for) && !TYPE_OC(judge_for)) {
            true_problem_id = strtoul(row[4],0,10); // 题目库id
        } else {
            true_problem_id = problem_id;
        }
        mysql_free_result(res);
        res = NULL;
    } else {
        warning("启动失败：%s\n",mysql_error(conn));
        return OJ_SE;
    }
#ifdef DEBUG
    printf("lang:%s pro_id:%u user_id:%u\n", LANG[lang], problem_id, user_id);
#endif // DEBUG
    if(!TYPE_OC(judge_for)) {
        return get_problem_info();
    }
    return 0;
}
