#ifndef JAVA_H_INCLUDED
#define JAVA_H_INCLUDED

#include "lang.h"

#define LANG_JAVA 3

extern unsigned memory_limit;

struct javaConfig: langConfig {
    javaConfig() {
        lang_id = 3; // 语言id
        useful = true; // 是否启用
        time_bonus = 2; // 标准时间奖励
        memoty_bonus = 2; // 标准内存奖励
        compile = true; // 是否需要编译 -> 需实现compile_cmd()
        compile_RLIMIT_AS = 0; //编译所需内存
        isPageMemory = true; // 是否是使用内存页
        needChmodXFile = true; // 是否需要转换为可运行文件 -> 需实现 chmodXFile()
        needFixError = true;
    }
    int compile_cmd() {
        return execlp("javac", "javac", "-J-Xms64m", "-J-Xmx256m","-encoding", "UTF-8", "-g:none", "-nowarn", "./Main.java", (char *)NULL);
    }
    int run_cmd() {
        char java_xms[64], java_xmx[64];
        sprintf(java_xms, "-Xms%dk", (memory_limit) + 1);
        sprintf(java_xmx, "-Xmx%dk", (memory_limit) + 2);
        return execl("/usr/bin/java", "/usr/bin/java", java_xms, java_xmx,"-Xss64m", "Main", (char *) NULL);
    }
    void chmodXFile() {
        if(system("chmod +x ./Main.class"));
    }
    int fixError() {
        if(!system("grep 'java.lang.OutOfMemoryError' ./error.out")) {
            return OJ_MLE;
        }
        return OJ_RE;
    }
};

#endif // JAVA_H_INCLUDED
