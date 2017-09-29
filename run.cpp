#include "solve.h"

char RUN_ROOM[64];
int run_id; // run in which room

unsigned status_id, judge_for;

//solve need
int time_limit, time_limit_second, memory_limit,time_used, memory_used;
int spj, judge_cnt, lang, true_problem_id;

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("judging need judge_id run_id\n");
        exit(1);
    }
    unsigned long long judge_id = strtoull(argv[1],0,10);
    status_id = (unsigned)(judge_id & 0xffffffff);
    judge_for = (unsigned)(judge_id >> 32);
    run_id = atoi(argv[2]);
    sprintf(RUN_ROOM, "%s%d", RUN_ROOM_PREFIX, run_id);
    execcmd("rm -f %s/*",RUN_ROOM);
    if(chdir(RUN_ROOM)) {
        warning("chdir to %s failed\n", RUN_ROOM);
        exit(1);
    }
    if(!judge_start()) {
        set_status(OJ_CI);
        set_status(solve());
    }
    close_mysql();
    return 0;
}

