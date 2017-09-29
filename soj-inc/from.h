#ifndef FROM_C_INCLUDED
#define FROM_C_INCLUDED

#include "support.h"
#include "compare.h"

extern MYSQL *conn;

extern char RUN_ROOM[64];

extern unsigned status_id, judge_for;
extern int time_limit, time_limit_second, memory_limit,time_used, memory_used;
extern int spj, judge_cnt, lang, true_problem_id;

int judge_start();
void set_status(unsigned);

#endif // FROM_C_INCLUDED
