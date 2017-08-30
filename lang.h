#ifndef LANG_H_INCLUDED
#define LANG_H_INCLUDED

#include "support.h"
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

#define STD_MB 1048576

#ifdef __x86_64__
#define SYSTEM_CALL orig_rax
#else
#define SYSTEM_CALL orig_eax
#endif // __x86_64__

extern long name;
extern unsigned int judge_for;
extern unsigned status_id, problem_id;
extern int lang, time_limit, memory_limit, time_used, memory_used, spj, judge_cnt, time_limit_second;
extern MYSQL *conn;

void write_file(const char *, const char *, ...) ;
long get_file_size(const char *) ;
int get_proc_status(int, const char *) ;
int run_test(int (*)(const char *)) ;

int c_solve() ;

#endif // LANG_H_INCLUDED
