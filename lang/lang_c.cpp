#include "../lang.h"

// sys/syscall.h -> asm/unistd.h -> asm/unistd_64.h
#ifdef __i386
int c_call[] = {-1};
#else
int c_call[] = {0, 1, 2, 3, 4, 5, 8, 9, 11, 12, 21, 59, 63, 158, 89, 201, 231, -1};
#endif

extern int system_call[512];

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

        if(chroot(RUN_PATH));

        while(setgid(POORUID) != 0) ;
        while(setuid(POORUID) != 0) ;
        while(setresuid(POORUID, POORUID, POORUID) != 0) ;

        if(freopen("./ce.txt", "w", stderr));
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

int c_solve() {
    if (c_compile()) return OJ_CE;
    for (int i = 0; c_call[i] != -1; i++) {
        system_call[c_call[i]] = -1;
    }
    run_test();
}
