#ifndef LANG_H_INCLUDED
#define LANG_H_INCLUDED

const long STD_MB = 1048576;

struct langConfig {
    short lang_id; // 语言id
    bool useful; // 是否启用
    short time_bonus; // 标准时间奖励
    short memoty_bonus; // 标准内存奖励
    bool compile; // 是否需要编译 -> 需实现compile_cmd()
    long compile_RLIMIT_AS; //编译所需内存,0为不限制
    bool isPageMemory; // 是否是使用内存页
    bool needChmodXFile; // 是否需要转换为可运行文件 -> 需实现 chmodXFile()
    long run_RLIMIT_NPROC; //最大进程数量
    bool needFixError; // 需要根据error文件获取具体运行错误 -> 需实现fixError
    virtual int compile_cmd(){}
    virtual void chmodXFile(){}
    virtual int run_cmd(){} // 运行命令
    virtual int fixError(){}
};

#endif // LANG_H_INCLUDED
