#include "lang.h"

int system_call[512];

void write_file(const char *cont, const char *fmt, ...) {
    char file_path[128];
    va_create(file_path);

    FILE *fp = fopen(file_path, "w");
    if(fp == NULL) {
        warning("open file error:%s\n", file_path);
        exit(1);
    }
    fprintf(fp, "%s", cont);
    fclose(fp);
}

long get_file_size(const char *file) {
    struct stat f_stat;
    stat(file, &f_stat);
    return f_stat.st_size;
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
    char buf[64];
    if(nice(19) == -1) {
        warning("nice is error");
    }
    while(setgid(POORUID) != 0) ;
    while(setuid(POORUID) != 0) ;
    while(setresuid(POORUID, POORUID, POORUID) != 0) ;

    sprintf(buf, "%s.in", path);
    if(freopen(buf, "r", stdin));

    sprintf(buf, "./%ld.out", name);
    if(freopen(buf, "w", stdout));
    if(freopen("error.out", "w", stderr));

    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    struct rlimit LIM;
    LIM.rlim_max = LIM.rlim_cur = time_limit_second + 1;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    // 防止sleep或其他
    alarm(time_limit_second + 2);

    // file limit 8MB
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

    // set the memory *1K * 4
    // unsigned long 免得溢出
    LIM.rlim_cur = (rlim_t)memory_limit << 12;
    LIM.rlim_max = (rlim_t)memory_limit << 12;
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
        if(WIFEXITED(status)) {
#ifdef DEBUG
            debug("normal exit %d\n", WTERMSIG(status));
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
                flag = OJ_FP;
                break;
            default:
                ptrace(PTRACE_KILL,pid,NULL,NULL);
                flag = OJ_TLE;
                break;
            }
            if(flag != OJ_AC){
#ifdef DEBUG
            debug("stop exit %d %s\n", sig, strsignal(sig));
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
            debug("sig exit:%d %s\n", sig, strsignal(sig));
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
                flag = OJ_FP;
                break;
            default:
                flag = OJ_RE;
            }
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
        if (regs.SYSTEM_CALL < 512 && system_call[regs.SYSTEM_CALL] != 0) {
#ifdef DEBUG
            debug("use system call:%llu\n", regs.SYSTEM_CALL);
#endif // DEBUG
        } else {
            warning("error:status_id:%u judge_for:%d not allow system call:%llu\n", status_id, judge_for, regs.SYSTEM_CALL);
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            flag = OJ_SC;
            break;
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
    return 0;
}

bool ispe(char c){
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

int check_ans(const char *pro,const char *user) {
    // if(spj) return check_ans_spj(pro, user);
    FILE *fp1, *fp2;
    int flag = OJ_AC,i,j, ii, jj;
#ifdef DEBUG
    debug("file:%s %s\n",pro,user);
#endif // DEBUG
    if((fp1= fopen(pro, "rb")) == NULL) {
        return OJ_AC;
    }
    if((fp2= fopen(user, "rb")) == NULL) {
        return OJ_WA;
    }
    int la = get_file_size(pro);
    int lb = get_file_size(user);
    char *p = (char*)malloc(sizeof(char)*la + 2);
    char *s = (char*)malloc(sizeof(char)*lb + 2);
    if(fread(p, 1, la + 1, fp1));
    if(fread(s, 1, lb + 1, fp2));
    for(i=0, j=0;i<la&&j<lb;i++,j++){
        if(p[i] == 13) i++;
        if(p[i]!=s[j]) {
            if(!ispe(p[i])&&!ispe(s[j])){
                free(p);
                free(s);
                return OJ_WA;
            }
            break;
        }
    }
    if(i==la&&j==lb){
        free(p);
        free(s);
        return OJ_AC;
    }
    for(ii=0;i<la;i++){
        while(ispe(p[i])) ++i;
        if(i==la) break;
        p[ii++] = p[i];
    }
    p[ii] = '\0';
    for(jj=0;j<lb;j++){
        while(ispe(s[j])) ++j;
        if(j==lb) break;
        s[jj++] = s[j];
    }
    s[jj] = '\0';
    if(!strcmp(s,p)) {
        flag = OJ_PE;
    } else {
        flag = OJ_WA;
    }
    free(p);
    free(s);
    return flag;
}

int run_test() {
    int f = OJ_AC, i, fal = OJ_AC;
    char buf[128],tbuf[128];
    for(i = 1; i <= judge_cnt; i++) {
        sprintf(buf, "%s/%d/pro%d_test%d", DATA_PATH, true_problem_id, true_problem_id, i);
        f = run(buf);
#ifdef DEBUG
        debug("running test %d , get status:%d\n",i,f);
#endif // DEBUG
        if(f != OJ_AC) {
            fal = f;
            break;
        }
        sprintf(buf, "%s.out", buf);
        sprintf(tbuf, "./%ld.out", name);
        f = check_ans(buf, tbuf);
#ifdef DEBUG
        debug("check test %d\n", f);
#endif // DEBUG
        if(f == OJ_WA) {
            fal = OJ_WA;
            break;
        }
        if(f == OJ_PE) {
            fal = OJ_PE;
            break;
        }
    }
    if(fal == OJ_AC) return OJ_AC;
    return fal * OJ_TEST_MAX + i;
}
