#ifdef __i386
int c_call[] = {3, 4, 11, 33, 45, 85, 122, 140, 197, 243, 252, -1};
#else
int c_call[] = {0, 1, 2, 3, 4, 5, 8, 9, 11, 12, 21, 59, 63, 89, 158, 201, 231, 240, -1};
#endif

extern int system_call[512];

int c_compile() {
    pid_t pid = fork();
    if (pid == 0) {
        compile_limit();
        if(freopen("./ce.txt", "w", stderr));
        char filename[64];
        if(lang == LANG_C) {
            sprintf(filename, "./%ld.c", name);
            if (execlp("gcc", "gcc", "-o", "Main", filename, "-std=c99","-lm", "-Wall", "--static",
                       "-std=c99", "-O2", "-DONLINE_JUDGE", (char *)NULL) == -1) {
                return 1;
            }
        } else {
            sprintf(filename, "./%ld.cpp", name);
            if (execlp("g++", "g++", "-o", "Main", filename, "-std=c++11", "-lm", "-Wall","--static",
                       "-O2", "-DONLINE_JUDGE", (char *)NULL) == -1) {
                return 1;
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


#ifdef __i386
int java_call[] = {3, 4, 5, 6, 11, 33, 45, 85, 91, 116, 120, 122, 125, 140, 174, 175, 191, 192, 195,
		 197, 220, 240, 243, 252, 258, 295, 311, -1};
#else
int java_call[] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 21, 39, 41, 42, 56, 59, 62, 63,
        77, 78, 79, 81, 83, 87, 89, 97, 99, 110, 111, 107, 158, 160, 201, 202, 204, 218, 229, 231, 240, 257, 273, -1};
#endif

extern int system_call[512];

int java_compile() {
    pid_t pid = fork();
    if (pid == 0) {
        struct rlimit LIM;
        // 用时10s
        LIM.rlim_cur = LIM.rlim_max = 10;
        setrlimit(RLIMIT_CPU, &LIM);
        // 加载文件16mb
        LIM.rlim_max =  LIM.rlim_cur = STD_MB << 4;
        setrlimit(RLIMIT_FSIZE, &LIM);
        if(chroot(RUN_PATH));
        while(setgid(POORUID) != 0) ;
        while(setuid(POORUID) != 0) ;
        while(setresuid(POORUID, POORUID, POORUID) != 0) ;
        if(freopen("./ce.txt", "w", stderr));
        if (execlp("javac", "javac", "-J-Xmx450m", "-J-Xms64m",
                   "-encoding", "UTF-8", "-g:none", "-nowarn", "./Main.java", (char *)NULL) == -1) {
            printf("nofind");
            return 1;
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

