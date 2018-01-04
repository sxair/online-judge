#ifndef CPP_H_INCLUDED
#define CPP_H_INCLUDED

#include "lang.h"

struct cppConfig: langConfig {
    cppConfig() {
        lang_id = 2; // 语言id
        useful = true; // 是否启用
        time_bonus = 1; // 标准时间奖励
        memoty_bonus = 1; // 标准内存奖励
        compile = true; // 是否需要编译 -> 需实现compile_cmd()
        compile_RLIMIT_AS = STD_MB << 9; //编译所需内存
        isPageMemory = false; // 是否是使用内存页
        needChmodXFile = false; // 是否需要转换为可运行文件 -> 需实现 chmodXFile()
        needFixError = false;
    }
    int compile_cmd() {
        return execlp("g++", "g++", "-o", "Main", "./Main.cpp", "-std=c++11", "-lm", "-Wall","--static",
                       "-O2", "-DONLINE_JUDGE", (char *)NULL);
    }
    int run_cmd() {
        return execl("./Main", "./Main", (char *) NULL);
    }
};

#endif // CPP_H_INCLUDED
