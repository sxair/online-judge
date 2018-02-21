#include "soj-inc/solve.h"

char RUN_ROOM[64];
int run_id; // run in which room
int judger_id;
unsigned status_id, judge_for;
unsigned time_limit, time_limit_second, memory_limit,time_used, memory_used;
int spj, judge_cnt, lang, true_problem_id;

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("judging need judge_id run_room\n");
        exit(1);
    }

    unsigned long long judge_id = strtoull(argv[1],0,10);
    status_id = (unsigned)(judge_id & 0xffffffff);
    judge_for = (unsigned)(judge_id >> 32);
    run_id = atoi(argv[2]);
    judge_id = JUDGER_BASE_ID + run_id;
    sprintf(RUN_ROOM, "%s/run%d", RUN_PATH, run_id);
    execcmd("rm -f %s/*",RUN_ROOM);
    if(chdir(RUN_ROOM)) {
        warning("chdir to %s failed\n", RUN_ROOM);
        exit(1);
    }
    //获取数据库信息
    if(judge_start()) {
        close_mysql();
        exit(OJ_SE);
    }

    set_lang_config();

    if(TYPE_OC(judge_for)) {
        // in only compile => status_id = problem_id
        set_oc_status(compile());
        delete_judge();
    } else {
        set_status(OJ_CI);
        set_status(solve());
        delete_judge();
    }
    /*if judge start faild. no delete judge*/
    close_mysql();
    return 0;
}

