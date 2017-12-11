#include "soj-inc/solve.h"

char RUN_ROOM[64];
int run_id; // run in which room

unsigned status_id, judge_for;

int time_limit, time_limit_second, memory_limit,time_used, memory_used;
int spj, judge_cnt, lang, true_problem_id;

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("judging need judge_id run_room\n");
        exit(1);
    }

    unsigned long long judge_id = strtoull(argv[1],0,10);
    status_id = (unsigned)(judge_id & 0xffffffff);
    judge_for = (unsigned)(judge_id >> 32);

    sprintf(RUN_ROOM, "%s/%s", RUN_PATH, argv[2]);
    execcmd("rm -f %s/*",RUN_ROOM);
    if(chdir(RUN_ROOM)) {
        warning("chdir to %s failed\n", RUN_ROOM);
        exit(1);
    }
    if(TYPE_OC(judge_for)) {
        // in only compile => status_id = problem_id
        if(execcmd("cp %s/%d/spj.cpp ./Main.cpp", DATA_PATH, status_id) == 0) {
            lang = LANG_CPP;
            set_oc_status(compile());
        } else {
            warning("only compile can't find source file:%d", status_id);
        }
        #ifndef DEBUG
            delete_judge();
        #endif // DEBUG
    } else if(!judge_start()) {
        set_status(OJ_CI);
        set_status(solve());
        //#ifndef DEBUG
            delete_judge();
        //#endif // DEBUG
    }
    /*if judge start faild. no delete judge*/
    close_mysql();
    return 0;
}

