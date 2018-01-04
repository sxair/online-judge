#include "soj-inc/solve.h"

int system_call[512];

#ifdef __i386
int allow_system_call[] = {3, 4, 5, 6, 11, 33, 45, 85, 91, 116, 120, 122, 125, 140, 174, 175, 191, 192, 195,
                           197, 220, 240, 243, 252, 258, 295, 311, -1
                          };
#else
int allow_system_call[] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 21, 39, 41, 42, 56, 59, 62, 63,
                           77, 78, 79, 81, 83, 87, 89, 97, 99, 110, 111, 107, 158, 160, 201, 202, 204, 218, 229, 231, 240, 257, 273, -1
                          };
#endif

langConfig *langCfg;

void set_lang_config() {
    switch(lang) {
    case LANG_C:
        langCfg = new cConfig();
        return;
    case LANG_CPP:
        langCfg = new cppConfig();
        return;
    case LANG_JAVA:
        langCfg = new javaConfig();
        return;
    case LANG_PY2:
        langCfg = new py2Config();
        return;
    case LANG_PY3:
        langCfg = new py3Config();
        return;
    }
    exit(OJ_SE);
}

int get_proc_status(int pid, const char *type) {
    FILE * fp;
    char path[64], buf[1024];
    int ans = 0, l = strlen(type);
    sprintf(path, "/proc/%d/status", pid);
    if((fp = fopen(path, "r")) != NULL) {
        while (fgets(buf, 1023, fp)) {
            if (strncmp(buf, type, l - 1) == 0) {
                sscanf(buf + l, "%d", &ans);
                fclose(fp);
                break;
            }
        }
    }
    return ans;
}

void run_judge(const char *path) {
    char buf[128];
    // 线程优先度最低
    if(nice(19) == -1) {
        warning("nice is error\n");
    }
    // 转为可执行文件
    if(langCfg->needChmodXFile) {
        langCfg->chmodXFile();
    }
    // 转换用户id
    while(setgid(POORUID) != 0) ;
    while(setuid(POORUID) != 0) ;
    while(setresuid(POORUID, POORUID, POORUID) != 0) ;

    sprintf(buf, "%s.in", path);
    if(freopen(buf, "r", stdin));
    if(freopen("user.out", "w", stdout));
    if(freopen("error.out", "w", stderr));
    //允许进程追踪
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    struct rlimit LIM;
    LIM.rlim_max = LIM.rlim_cur = time_limit_second + 1;
    setrlimit(RLIMIT_CPU, &LIM);
    // 防止sleep或其他
    alarm(time_limit_second << 3);

    // // file limit 8MB
    if(spj) {
        LIM.rlim_max = LIM.rlim_cur = STD_MB << 3;
    } else {
        sprintf(buf, "%s.out", path);
        LIM.rlim_max = LIM.rlim_cur = get_file_size(buf) << 1;
    }
    setrlimit(RLIMIT_FSIZE, &LIM);

    // set the stack 64MB
    LIM.rlim_cur = STD_MB << 6;
    LIM.rlim_max = STD_MB << 6;
    setrlimit(RLIMIT_STACK, &LIM);

    // RLIMIT_AS-> byte, so set the memory *1K + 1MB
    // unsigned long 免得溢出.其他语言一直卡不了。。
    if(lang == LANG_C || lang == LANG_CPP) {
        LIM.rlim_cur = ((rlim_t)memory_limit << 10) + STD_MB;
        LIM.rlim_max = ((rlim_t)memory_limit << 10) + STD_MB;
        setrlimit(RLIMIT_AS, &LIM);
    }

    LIM.rlim_cur = LIM.rlim_max = langCfg->run_RLIMIT_NPROC;
    setrlimit(RLIMIT_NPROC, &LIM);

    if(langCfg->run_cmd() == -1) {
        warning("运行失败，语言编号：%d\n", lang);
        exit(OJ_SE);
    }

    fflush(stdout);
    fflush(stderr);
    exit(0);
}

int watch_judge(pid_t pid) {
    int tmp_mem;
    int sig, flag = OJ_AC;
    unsigned status;
    struct user_regs_struct regs;
    struct rusage rus;
    memory_used = get_proc_status(pid, "VmRSS:");
    // http://blog.csdn.net/edonlii/article/details/8717029
    while(1) {
        wait4(pid, &status, 0, &rus);

        tmp_mem = langCfg->isPageMemory ? rus.ru_minflt * (getpagesize() >> 10) : get_proc_status(pid, "VmPeak:");

        if(memory_used < tmp_mem) memory_used = tmp_mem;
        if(memory_used > memory_limit) {
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            flag = OJ_MLE;
            break;
        }
        if(WIFEXITED(status)) {
#ifdef DEBUG
            printf("normal exit %d\n", WTERMSIG(status));
#endif // DEBUG
            break;
        }
        if(WIFSTOPPED(status)) {
            sig = WSTOPSIG(status);
            switch(sig) {
            case SIGTRAP:// 追踪信号
                break;
            case SIGXFSZ: // file size
                flag = OJ_OLE;
                break;
            case SIGSEGV: //Segmentation fault
                flag = OJ_SF;
                break;
            case SIGFPE: //Floating Point Exception
                flag = OJ_FPE;
                break;
            default:
                ptrace(PTRACE_KILL,pid,NULL,NULL);
                flag = OJ_TLE;
                break;
            }
            if(flag != OJ_AC) {
#ifdef DEBUG
                printf("stop exit %d %s\n", sig, strsignal(sig));
#endif // DEBUG
                break;
            }
        }
        // 信号退出
        if (WIFSIGNALED(status)) {
            sig = WTERMSIG(status);
            // http://blog.csdn.net/stormkey/article/details/5890512
            // man setrlimit
#ifdef DEBUG
            printf("sig exit:%d %s\n", sig, strsignal(sig));
#endif // DEBUG
            switch (sig) {
            case SIGCHLD:
            case SIGALRM: // alarm is end
            case SIGKILL:
            case SIGXCPU: // RLIMIT_CPU time end
                flag = OJ_TLE;
                break;
            case SIGXFSZ: // file size
                flag = OJ_OLE;
                break;
            case SIGSEGV: //Segmentation fault
                flag = OJ_SF;
                break;
            case SIGFPE: //Floating Point Exception
                flag = OJ_FPE;
                break;
            default:
                flag = OJ_RE;
            }
            break;
        }
        if(get_file_size("error.out")) {
#ifdef DEBUG
            printf("检测到错误文件\n");
#endif // DEBUG
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            flag = langCfg->needFixError ? langCfg->fixError(): OJ_RE;
            break;
        }
        //读取系统调用
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);
        if (regs.SYSTEM_CALL < 512 && system_call[regs.SYSTEM_CALL] != 0) {
#ifdef DEBUG
            //   printf("use system call:%llu\n", regs.SYSTEM_CALL);
#endif // DEBUG
        } else {
            warning("error:status_id:%u judge_for:%d not allow system call:%d\n", status_id, judge_for, regs.SYSTEM_CALL);
#ifndef DEBUG
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            flag = OJ_NASC;
            break;
#endif
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }
    int t_time = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000 + rus.ru_stime.tv_sec * 1000 + rus.ru_stime.tv_usec / 1000;
    if(t_time > time_used)
        time_used = t_time;

    if(flag == OJ_TLE || time_used > time_limit) {
        time_used = time_limit;
        return OJ_TLE;
    }

    if(flag == OJ_MLE || memory_used > memory_limit) {
        memory_used = memory_limit;
        return OJ_MLE;
    }
    return flag;
}

int run(const char * buf) {
    pid_t pid = fork();
    if(pid == 0) {
        run_judge(buf);
    } else {
        return watch_judge(pid);
    }
    exit(0);
}

/*
* 运行每个测试样例并判断，出现不是ac的则退出
*/
unsigned run_test() {
    unsigned f = OJ_AC, i, fal = OJ_AC;
    char buf[128];
    for(i = 1; i <= judge_cnt; i++) {
        set_status(OJ_RI * MAX_TEST + i);
        sprintf(buf, "%s/%d/pro%d_test%d", DATA_PATH, true_problem_id, true_problem_id, i);
        f = run(buf);
#ifdef DEBUG
        printf("running test %d, get status:%d\n",i,f);
#endif // DEBUG
        if(f != OJ_AC) {
            fal = f;
            break;
        }
        sprintf(buf, "%s.out", buf);
        f = check_ans(buf, "user.out", spj);
#ifdef DEBUG
        printf("check test %d\n", f);
#endif // DEBUG
        if(f != OJ_AC) {
            fal = f;
            break;
        }
    }
    if(fal == OJ_AC) return OJ_AC;
    return fal * MAX_TEST + i;
}

int compile() {
    pid_t pid = fork();
    if (pid == 0) {
        struct rlimit LIM;
        // 用时10s
        LIM.rlim_cur = LIM.rlim_max = 10;
        setrlimit(RLIMIT_CPU, &LIM);
        // 编译输出文件16mb
        LIM.rlim_max =  LIM.rlim_cur = STD_MB << 4;
        setrlimit(RLIMIT_FSIZE, &LIM);
        // 内存限制
        if(langCfg->compile_RLIMIT_AS != 0) {
            LIM.rlim_max = LIM.rlim_cur = langCfg->compile_RLIMIT_AS;
            setrlimit(RLIMIT_AS, &LIM);
        }
        //转换运行地址，防止include攻击
        if(chroot(RUN_PATH));

        if(setgid(POORUID) != 0) return 1;
        if(setuid(POORUID) != 0) return 1;
        if(setresuid(POORUID, POORUID, POORUID) != 0) return 1;

        if(freopen("./ce.txt", "w", stderr));
        if(langCfg->compile_cmd() == -1) {
            warning("编译失败，语言编号：%d\n", lang);
            exit(OJ_SE);
        }
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
#ifdef DEBUG
        printf("compile status:%d\n", status);
#endif // DEBUG
        return status;
    }
}

int solve() {
    if (langCfg->compile && compile()) return OJ_CE;
    memory_limit *= langCfg->memoty_bonus;
    time_limit *= langCfg->time_bonus;
    for (int i = 0; allow_system_call[i] != -1; i++) {
        system_call[allow_system_call[i]] = -1;
    }
    return run_test();
}

