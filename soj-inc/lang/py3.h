#ifndef PY3_H_INCLUDED
#define PY3_H_INCLUDED

#include "lang.h"

struct py3Config: langConfig {
    py3Config() {
        lang_id = 5; // 语言id
        useful = true; // 是否启用
        time_bonus = 2; // 标准时间奖励
        memoty_bonus = 2; // 标准内存奖励
        compile = false; // 是否需要编译 -> 需实现compile_cmd()
        compile_RLIMIT_AS = 0; //编译所需内存
        isPageMemory = false; // 是否是使用内存页
        needChmodXFile = false; // 是否需要转换为可运行文件 -> 需实现 chmodXFile()
        needFixError = false;
    }
    int run_cmd() {
        return execl("/usr/bin/python3", "python3", "./Main.py", (char *) NULL);
    }
};


#endif // PY3_H_INCLUDED
