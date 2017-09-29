#ifndef SOLVE_H_INCLUDED
#define SOLVE_H_INCLUDED

#include "support.h"
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

int solve();

#endif // SOLVE_H_INCLUDED
