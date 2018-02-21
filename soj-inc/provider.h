#ifndef FROM_C_INCLUDED
#define FROM_C_INCLUDED

#include "support.h"
#include "compare.h"

//one contest status no more than 268435456-1
// judge_for == 0 is oj_problem
// only complie => like oj_problem
// judge_for != 0 then status_id => contest_id,  judge_for^base => contest_status_id
#define CONTEST_BASE 0x10000000
#define DIY_BASE     0x20000000
#define OC_BASE      3 //only compile
#define ADMIN_BASE   4

#define TYPE_OJ(judge_for) (judge_for==0)
#define TYPE_CONTEST(judge_for) ((judge_for&CONTEST_BASE)==CONTEST_BASE)
#define TYPE_DIY(judge_for) ((judge_for&DIY_BASE)==DIY_BASE)
#define TYPE_ADMIN(judge_for) ((judge_for&ADMIN_BASE)==ADMIN_BASE)
#define TYPE_OC(judge_for) ((judge_for&OC_BASE)==OC_BASE)

#define CONTEST_STATUS_ID(judge_for) (judge_for^CONTEST_BASE)
#define DIY_STATUS_ID(judge_for) (judge_for^DIY_BASE)

extern MYSQL *conn;

extern char RUN_ROOM[64];
extern int judger_id;
extern unsigned status_id, judge_for;
extern unsigned time_limit, time_limit_second, memory_limit,time_used, memory_used;
extern int spj, judge_cnt, lang, true_problem_id;

int judge_start();
void set_status(unsigned);
void set_oc_status(int);
void delete_judge();
#endif // FROM_C_INCLUDED
