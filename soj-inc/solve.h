#ifndef SOLVE_H_INCLUDED
#define SOLVE_H_INCLUDED

#include "support.h"
#include "provider.h"
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

// 如新增语言记得加入头文件
#include "lang/c.h"
#include "lang/cpp.h"
#include "lang/java.h"
#include "lang/py2.h"
#include "lang/py3.h"

extern const long STD_MB;

#ifdef __x86_64__
#define SYSTEM_CALL orig_rax
#else
#define SYSTEM_CALL orig_eax
#endif // __x86_64__

/**
* 设置语言
*/
void set_lang_config();

/**
* 读取当前 Main.lang(后缀) 文件，并根据lang参数去进行编译
* lang参数在provider.h中
* 返回 ！=0 为编译失败
* 成功后在本地生成 Main 文件
*/
int compile();

int solve();

#endif // SOLVE_H_INCLUDED
