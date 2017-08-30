#include "../lang.h"

// sys/syscall.h -> asm/unistd.h -> asm/unistd_64.h
#ifdef __i386
int c_call[] = {-1};
#else
int c_call[] = {0, 1, 2, 3, 4, 5, 8, 9, 11, 12, 21, 59, 63, 158, 89, 201, 231, -1};
#endif

int c_system_call[512];

int c_compile() {
    pid_t pid = fork();
    if (pid == 0) {
        struct rlimit LIM;
        // 用时10s
        LIM.rlim_cur = LIM.rlim_max = 10;
        setrlimit(RLIMIT_CPU, &LIM);

        // 加载文件16mb
        LIM.rlim_max =  LIM.rlim_cur = STD_MB << 4;
        setrlimit(RLIMIT_FSIZE, &LIM);

        // 虚拟内存 256mb
        LIM.rlim_max = STD_MB << 8;
        LIM.rlim_cur = STD_MB << 8;
        setrlimit(RLIMIT_AS, &LIM);

        chroot(RUN_PATH);
        while(setgid(POORUID) != 0) ;
        while(setuid(POORUID) != 0) ;
        while(setresuid(POORUID, POORUID, POORUID) != 0) ;

        freopen("./ce.txt", "w", stderr);
        char filename[64];

        if(lang == 0) {
            sprintf(filename, "./%ld.c", name);
            if (execlp("gcc", "gcc", "-o", "Main", filename, "-lm", "-W", "--static",
                   "-std=c99", "-O2", "-DONLINE_JUDGE", "-fno-asm", (char *)NULL) == -1) {
                   exit(1);
            }
        } else {
            sprintf(filename, "./%ld.cpp", name);
            if (execlp("g++", "g++", "-o", "Main", filename, "-lm", "-W","--static",
                   "-x", "c++", "-O2", "-DONLINE_JUDGE", "-fno-asm", (char *)NULL) == -1) {
                   exit(1);
                   }
        }
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
#ifdef DEBUG
        debug("compile status:%d\n", status);
#endif // DEBUG
        return status;
    }
}

void run_judge(const char * input_path) {
    char outfile[64];
    sprintf(outfile, "%ld.out", name);
    nice(19);
    while(setgid(POORUID) != 0) ;
    while(setuid(POORUID) != 0) ;
    while(setresuid(POORUID, POORUID, POORUID) != 0) ;

    freopen(input_path, "r", stdin);
    freopen(outfile, "w", stdout);
    freopen("error.out", "w", stderr);

    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    struct rlimit LIM;
    LIM.rlim_max = LIM.rlim_cur = time_limit_second + 1;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    // 3 倍时间 防止sleep或其他
    alarm(time_limit_second * 3);

    // file limit 8MB
    LIM.rlim_max = LIM.rlim_cur = STD_MB << 3;
    setrlimit(RLIMIT_FSIZE, &LIM);

    // set the stack 64MB
    LIM.rlim_cur = STD_MB << 6;
    LIM.rlim_max = STD_MB << 6;
    setrlimit(RLIMIT_STACK, &LIM);

    // set the memory *1K * 4
    LIM.rlim_cur = memory_limit << 12;
    LIM.rlim_max = memory_limit << 12;
    setrlimit(RLIMIT_AS, &LIM);

    // proc limit 1 for execl
    LIM.rlim_cur = LIM.rlim_max = 1;
    setrlimit(RLIMIT_NPROC, &LIM);

    execl("./Main", "./Main", (char *) NULL);
    fflush(stdout);
    fflush(stderr);
    exit(0);
}

int watch_judge(pid_t pid) {
    int tmp_mem;
    int status, sig, flag = OJ_AC;
    struct user_regs_struct regs;
    struct rusage rus;
    memory_used = get_proc_status(pid, "VmRSS:");
    //memory_used = rus.ru_maxrss;
    while(1) {
        wait4(pid, &status, 0, &rus);

        tmp_mem = get_proc_status(pid, "VmPeak:");
        //tmp_mem = rus.ru_maxrss;
        if(memory_used < tmp_mem) memory_used = tmp_mem;
        if(memory_used > memory_limit) {
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            flag = OJ_MLE;
            break;
        }
        sig = WSTOPSIG(status);
        if(!sig) {
#ifdef DEBUG
            debug("normal exit\n");
#endif // DEBUG
            break;
        }
        if(WIFSTOPPED(status)) {
            switch(sig) {
            case SIGTRAP:// 追踪信号
                break;
            default:
                ptrace(PTRACE_KILL,pid,NULL,NULL);
                flag = OJ_TLE;
                break;
            }
        }
        // 信号退出
        if (WIFSIGNALED(status)) {

            sig = WTERMSIG(status);
            // http://blog.csdn.net/stormkey/article/details/5890512
            // man setrlimit
#ifdef DEBUG
            debug("sig exit:%d\n", sig);
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
//            case SIGSEGV: // STACK_OVERFLOW
//                flag = OJ_OF;
//                break;
            case SIGSEGV: //ACCESS_VIOLATION
                flag = OJ_AS;
                break;
            case SIGFPE: //INTEGER_DIVIDE_BY_ZERO
                flag = OJ_DZ;
                break;
            default:
                flag = OJ_RE;
            }
#ifdef DEBUG
            debug("error:status_id:%u sig:%s", status_id, strsignal(sig));
#endif // DEBUG
            break;
        }
        if(get_file_size("error.out")) {
        #ifdef DEBUG
                    debug("检测到错误文件\n");
        #endif // DEBUG
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            flag = OJ_RE;
            break;
        }
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);

        if (c_system_call[regs.SYSTEM_CALL] != 0) {
            //debug("use system call:%llu\n", regs.orig_rax);
        } else {
            warning("error:status_id:%u judge_for:%d not allow system call:%llu\n", status_id, judge_for, regs.SYSTEM_CALL);
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            flag = OJ_RE;
            break;
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }
    time_used = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
    time_used += rus.ru_stime.tv_sec * 1000 + rus.ru_stime.tv_usec / 1000;

    if(time_used > time_limit) {
        time_used = time_limit;
        return OJ_TLE;
    }
    return flag;
}

int c_run(const char * buf) {
    pid_t pid = fork();
    if(pid == 0) {
        run_judge(buf);
    } else {
        return watch_judge(pid);
    }
    return 0;
}

int c_solve() {
    if (c_compile()) return OJ_CE;
    for (int i = 0; c_call[i] != -1; i++) {
        c_system_call[c_call[i]] = -1;
    }
    run_test(c_run);
}
