#ifndef SOLVE_H_INCLUDED
#define SOLVE_H_INCLUDED

#include "support.h"
#include "compare.h"
#include "lang.h"
#include "from.h"
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

#define STD_MB 1048576

#ifdef __x86_64__
#define SYSTEM_CALL orig_rax
#else
#define SYSTEM_CALL orig_eax
#endif // __x86_64__

extern int time_limit, time_limit_second, memory_limit,time_used, memory_used;
extern int spj, judge_cnt, lang, true_problem_id;

int solve();

#endif // SOLVE_H_INCLUDED
