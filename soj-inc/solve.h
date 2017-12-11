#ifndef SOLVE_H_INCLUDED
#define SOLVE_H_INCLUDED

#include "support.h"
#include "provider.h"
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

#define STD_MB 1048576

#ifdef __x86_64__
#define SYSTEM_CALL orig_rax
#else
#define SYSTEM_CALL orig_eax
#endif // __x86_64__

/*
* 读取当前 Main.lang(后缀) 文件，并根据lang参数去进行编译
* lang参数在provider.h中
* 返回 ！=0 为编译失败
* 成功后在本地生成 Main 文件
*/
int compile();

int solve();

#endif // SOLVE_H_INCLUDED
