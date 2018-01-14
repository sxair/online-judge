#ifndef PY2_H_INCLUDED
#define PY2_H_INCLUDED

#include "lang.h"

#define LANG_PY2 4

struct py2Config: langConfig {
    py2Config() {
        lang_id = 4; // 语言id
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
        return execl("/usr/bin/python", "python", "./Main.py", (char *) NULL);
    }
};

#endif // PY2_H_INCLUDED
