#include "soj-inc/solve.h"

int system_call[512];

#ifdef __i386
int c_call[] = {3, 4, 11, 33, 45, 85, 122, 140, 197, 243, 252, -1};
int java_call[] = {3, 4, 5, 6, 11, 33, 45, 85, 91, 116, 120, 122, 125, 140, 174, 175, 191, 192, 195,
		 197, 220, 240, 243, 252, 258, 295, 311, -1};
int py_call[] = {3, 4, 5, 6, 11, 33, 45, 85, 91, 116, 120, 122, 125, 140, 174, 175, 191, 192, 195,
		 197, 220, 240, 243, 252, 258, 295, 311, -1};
#else
int c_call[] = {0, 1, 2, 3, 4, 5, 8, 9, 11, 12, 21, 59, 63, 89, 158, 201, 231, 240, -1};
int java_call[] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 21, 39, 41, 42, 56, 59, 62, 63,
        77, 78, 79, 81, 83, 87, 89, 97, 99, 110, 111, 107, 158, 160, 201, 202, 204, 218, 229, 231, 240, 257, 273, -1};
int py_call[] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 21, 39, 41, 42, 56, 59, 62, 63,
        77, 78, 79, 81, 83, 87, 89, 97, 99, 110, 111, 107, 158, 160, 201, 202, 204, 218, 229, 231, 240, 257, 273, -1};
#endif

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
    if(nice(19) == -1) {
        warning("nice is error");
    }
    if(lang == LANG_JAVA) {
        execcmd("chmod +x ./Main.class");
    }
    while(setgid(POORUID) != 0) ;
    while(setuid(POORUID) != 0) ;
    while(setresuid(POORUID, POORUID, POORUID) != 0) ;

    sprintf(buf, "%s.in", path);
    if(freopen(buf, "r", stdin));
    if(freopen("user.out", "w", stdout));
    if(freopen("error.out", "w", stderr));

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

    // set the memory *1K + 1K
    // unsigned long 免得溢出
    if(lang != LANG_JAVA) {
	    LIM.rlim_cur = ((rlim_t)memory_limit << 10) + 1024;
	    LIM.rlim_max = ((rlim_t)memory_limit << 10) + 1024;
        setrlimit(RLIMIT_AS, &LIM);
	}

    LIM.rlim_cur = LIM.rlim_max = 1;
    if(lang == LANG_JAVA) {
    	LIM.rlim_cur = LIM.rlim_max = 80;
    }
    setrlimit(RLIMIT_NPROC, &LIM);

    if(lang == LANG_C || lang == LANG_CPP) {
        execl("./Main", "./Main", (char *) NULL);
    } else if(lang == LANG_JAVA) {
        char java_xms[64], java_xmx[64];
        sprintf(java_xms, "-Xms%dm", (memory_limit >> 10) + 1);
        sprintf(java_xmx, "-Xmx%dm", (memory_limit >> 10) + 2);
        if(execl("/usr/bin/java", "/usr/bin/java", java_xms, java_xmx, "-Xss64m", "Main", (char *) NULL)==-1){
            printf("java run wrong");
        }
    } else if(lang == LANG_PY2){
        if(execl("/usr/bin/python", "./Main.py", (char *) NULL)==-1){
            printf("python run wrong");
        }
    } if(lang == LANG_PY3){
        if(execl("/usr/bin/python3", "./Main.py", (char *) NULL)==-1){
            printf("python run wrong");
        }
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
    while(1) {
        wait4(pid, &status, 0, &rus);
        if(lang == LANG_JAVA) {
        	//java use page memory
        	//ru_minflt -> miss page time
        	//getpagesize -> Byte
            tmp_mem = rus.ru_minflt * (getpagesize() >> 10);
        } else {
            tmp_mem = get_proc_status(pid, "VmPeak:");
        }
        //tmp_mem = rus.ru_maxrss;
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
            flag = OJ_RE;
            break;
        }
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
    time_used = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
    time_used += rus.ru_stime.tv_sec * 1000 + rus.ru_stime.tv_usec / 1000;

    if(flag == OJ_TLE || time_used > time_limit) {
        time_used = time_limit;
        return OJ_TLE;
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

        // 加载文件16mb
        LIM.rlim_max =  LIM.rlim_cur = STD_MB << 4;
        setrlimit(RLIMIT_FSIZE, &LIM);

        // java虚拟内存 512mb .但不知为什么设为11才不报错。。。。
        if(lang == LANG_JAVA) {
            LIM.rlim_max = STD_MB << 11;
            LIM.rlim_cur = STD_MB << 11;
        } else {
            LIM.rlim_max = STD_MB << 8;
            LIM.rlim_cur = STD_MB << 8;
            setrlimit(RLIMIT_AS, &LIM);
        }

        if(chroot(RUN_PATH));

        if(setgid(POORUID) != 0) return 1;
        if(setuid(POORUID) != 0) return 1;
        if(setresuid(POORUID, POORUID, POORUID) != 0) return 1;

        if(freopen("./ce.txt", "w", stderr));
        if(lang == LANG_C) {
            if (execlp("gcc", "gcc", "-o", "Main", "./Main.c", "-std=c99","-lm", "-Wall", "--static",
                       "-std=c99", "-O2", "-DONLINE_JUDGE", (char *)NULL) == -1) {
                exit(1);
            }
        } else if(lang == LANG_CPP){
            if (execlp("g++", "g++", "-o", "Main", "./Main.cpp", "-std=c++11", "-lm", "-Wall","--static",
                       "-O2", "-DONLINE_JUDGE", (char *)NULL) == -1) {
                exit(1);
            }
        } else if(lang == LANG_JAVA){
            if (execlp("javac", "javac", "-encoding", "UTF-8", "-g:none", "-nowarn", "./Main.java", (char *)NULL) == -1) {
                exit(1);
            }
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
    if(lang != LANG_PY2 && lang != LANG_PY3) {
        if (compile()) return OJ_CE;
    }
    if(lang == LANG_C || lang == LANG_CPP) {
        for (int i = 0; c_call[i] != -1; i++) {
            system_call[c_call[i]] = -1;
        }
    } else if(lang == LANG_JAVA) {
        for (int i = 0; java_call[i] != -1; i++) {
            system_call[java_call[i]] = -1;
        }
        memory_limit <<= 1;
        time_limit <<=1;
        time_limit_second <<= 1;
    } else if(lang == LANG_PY2 || lang == LANG_PY3) {
        for (int i = 0; py_call[i] != -1; i++) {
            system_call[py_call[i]] = -1;
        }
        memory_limit <<= 2;
        time_limit <<=2;
        time_limit_second <<= 2;
    }
    return run_test();
}

