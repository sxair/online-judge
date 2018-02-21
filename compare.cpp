#include "soj-inc/compare.h"

/**
* p1 为buf 头，p2 尾
* 如果p1 == p2 说明buf区使用完。要用fread读取，读取完后判断p1==p2来返回
* 逗号运算符：值为最后一个。
*/
char buf[100000],*p1=buf,*p2=buf;
inline char nc(FILE *f) {
    return p1==p2&&(p2=(p1=buf)+fread(buf,1,100000,f),p1==p2)?EOF:*p1++;
}
inline char nc1(FILE *f) {
    char t = nc(f);
    if(t == 13) t = nc(f);
    return t;
}

char buf2[100000],*sf=buf2,*ef=buf2;
inline char nc2(FILE *f) {
    return sf==ef&&(ef=(sf=buf2)+fread(buf2,1,100000,f),sf==ef)?EOF:*sf++;
}

bool ispe(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

void run_compare(const char *pro,const char *user) {
    char buf[128];
    // 转换用户id
    while(setgid(judger_id) != 0) ;
    while(setuid(judger_id) != 0) ;
    while(setresuid(judger_id, judger_id, judger_id) != 0) ;

    //if(freopen("spj.out", "w", stdout));

    struct rlimit LIM;
    LIM.rlim_max = LIM.rlim_cur = 10;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(10);

    // file limit 8MB
    LIM.rlim_max = LIM.rlim_cur = STD_MB << 3;
    setrlimit(RLIMIT_FSIZE, &LIM);

    // set the stack 64MB
    LIM.rlim_cur = STD_MB << 6;
    LIM.rlim_max = STD_MB << 6;
    setrlimit(RLIMIT_STACK, &LIM);

    LIM.rlim_cur = STD_MB << 7;
    LIM.rlim_max = STD_MB << 7;
    setrlimit(RLIMIT_AS, &LIM);
    int l = strlen(pro);
    strncpy(buf, pro, l - 3);
    buf[l - 3] = 0;
    sprintf(buf, "%sin", buf);
    if(execl("./spj", "./spj", pro, user, buf,(char *) NULL) == -1) {
        warning("运行失败，语言编号：%d\n", lang);
        exit(OJ_SE);
    }

    //fflush(stdout);
    exit(0);
}

int watch_compare(pid_t pid) {
    int status;
    waitpid(pid, &status, 0);
    if(WIFEXITED(status)) {
        return WEXITSTATUS(status) == 0 ? OJ_AC : OJ_WA;
    }
    return OJ_WA;
}

int check_ans_spj(const char *pro,const char *user) {
    execcmd("cp -f %s/%d/spj ./spj", DATA_PATH, true_problem_id);
    pid_t pid = fork();
    if(pid == 0) {
        run_compare(pro, user);
    } else {
        return watch_compare(pid);
    }
    exit(0);
}

int check_ans(const char *pro,const char *user, int spj) {
    if(spj) return check_ans_spj(pro, user);
    FILE *fp1, *fp2;
#ifdef DEBUG
    printf("file:%s %s\n",pro,user);
#endif // DEBUG
    if((fp1 = fopen(pro, "rb")) == NULL) {
        return OJ_SE;
    }
    if((fp2= fopen(user, "rb")) == NULL) {
        return OJ_WA;
    }
    int flag = OJ_AC;
    char c1,c2;
    while(c1=nc1(fp1), c2=nc2(fp2), c1 !=EOF && c2 != EOF) {
        if(c1 != c2) {
            if(!ispe(c1) && !ispe(c2)) { // 如果两个都不是pe字符。则wa
                flag = OJ_WA;
            } else flag = OJ_PE;
            break;
        }
    }
    if(flag == OJ_AC) {
        if(c1 != c2) flag = OJ_PE; //如果还有一个不是EOF
    }
    if(flag != OJ_PE) return flag;
    while(c1 != EOF || c2 != EOF) {
        // 去除pe字符
        while(ispe(c1) && (c1 = nc1(fp1)) != EOF) ;
        while(ispe(c2) && (c2 = nc2(fp2)) != EOF) ;
        if(c1 != c2) return OJ_WA;
        c1 = nc1(fp1);
        c2 = nc2(fp2);
    }
    return OJ_PE;
}
