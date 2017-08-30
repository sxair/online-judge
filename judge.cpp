#include "support.h"
#include <fcntl.h>

extern MYSQL *conn;

void set_file() {
    execcmd("umount %s/usr %s/bin %s/lib", RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH);
#ifdef __x86_64__
    execcmd("umount %s/lib64", RUN_PATH);
#endif // __x86_64__
    execcmd("mkdir -p %s %s %s %s %s", HOME_PATH, LOG_PATH, RUN_PATH, PID_PATH, DATA_PATH);
    for (int i = 0; i < RUN_MAX; i++) {
        execcmd("mkdir -p %s/run%d", RUN_PATH, i);
        execcmd("chown judge:judge %s/run%d", RUN_PATH, i);
    }
    execcmd("mkdir -p %s/usr %s/bin %s/lib ", RUN_PATH, RUN_PATH, RUN_PATH);
#ifdef __x86_64__
    execcmd("mkdir -p %s/lib64",RUN_PATH);
#endif // __x86_64__
    // remember to umount!
    execcmd("mount --bind /usr %s/usr",RUN_PATH);
    execcmd("mount --bind /bin %s/bin",RUN_PATH);
    execcmd("mount --bind /lib %s/lib",RUN_PATH);
#ifdef __x86_64__
    execcmd("mount --bind /lib64 %s/lib64",RUN_PATH);
#endif // __x86_64__
}

void chk_running() {
    char buf[64];
    sprintf(buf, "%s/judge.pid", PID_PATH);
    // O:读写。不存在则新建 创建时：S（受unmask限制）:用户读写，组，其他可读
    int fd = open(buf, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        warning("oj already running.\n");
        exit(1);
    }
    struct flock fl;
    fl.l_len = 0;
    fl.l_start = 0;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        warning("lock %s failed:%s",buf,strerror(errno));
        exit(1);
    }
    if(!ftruncate(fd, 0)){ // clear content
        sprintf(buf, "%d", getpid());
        int t = write(fd, buf, strlen(buf));
    }
    close(fd);
}

void daemon(void) {
    pid_t pid;
    while((pid = fork())< 0) {
        printf("daemon ceate faild\n");
    }
    if (pid != 0) exit(0); // father eixt
    printf("daemon ceate success\n oj running now\n");
    setsid(); // give son a id
    printf("running in %d\n",getpid());
    umask(0); // if create file then 777
    close(0);
    close(1);
    close(2);
    // for some fnc
    int fd = open( "/dev/null", O_RDWR );
    dup2( fd, 0 );
    dup2( fd, 1 );
    dup2( fd, 2 );
    if ( fd > 2 ) {
        close( fd );
    }
    return ;
}

int STOP;
void stop_judge(int s) {
    STOP = 1;
#ifdef DEBUG
    debug("\nIf nothing judged will exit.\nPlease wait\n");
#endif // DEBUG
}

void run(unsigned long long judge_id, int run_id) {
    char judgeid[68], runid[68];
    struct rlimit LIM;
    LIM.rlim_max = OJ_RUN_TIME; // second
    LIM.rlim_cur = OJ_RUN_TIME;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(OJ_RUN_TIME);
    sprintf(judgeid, "%llu",judge_id);
    sprintf(runid, "%d", run_id);
    if (execl("./run", "./run", judgeid, runid, (char *)NULL) == -1) {
        warning("error:run程序无法运行\n");
    }
}

int queue_top, queue_last, run_cnt;
unsigned long long judge_queue[RUN_MAX << 1 | 1], running[RUN_MAX];
pid_t pid[RUN_MAX + 1], tpid;

void set_judges() {
    if(!connect_mysql()) {
        return ;
    }
    queue_top = 0;
    execsql("select status_id,judge_for from judges limit %d", RUN_MAX << 1);
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;
    if(res != NULL) {
        while ((row = mysql_fetch_row(res)) != NULL) {
            judge_queue[queue_top] = atoi(row[0]);
            judge_queue[queue_top++] |= atoll(row[1])<<32;
        }
        mysql_free_result(res);
        res = NULL;
    }
    if(!run_cnt) return ;

    // 为了不重复判断。一个服务器只开启一个judge也是因为如此
    // 若要多服务器判题，可以对judge_id进行划分（%2） 或分表等
    for(int i = 0; i < queue_top; i++) {
        for(int j = 0; j < RUN_MAX; j++) {
            if(judge_queue[i] == running[j]) {
                judge_queue[i] = 0;
                break;
            }
        }
    }
}
unsigned long long get_judge() {
    for(; queue_last < queue_top; queue_last++) {
        if(judge_queue[queue_last]) {
            return judge_queue[queue_last++];
        }
    }
    set_judges();
    return queue_last = 0;
}

void start_judge() {
    int i, status;
    unsigned long long t;
    // 获取初始待评判队列
    set_judges();
    while(1) {
        // 获取空闲位置
        if(run_cnt < RUN_MAX) {
            for(i = 0; pid[i]; i++) ;
        } else {
            tpid = waitpid(-1, &status, 0);
#ifdef DEBUG
            debug("pid = %d status:%d\n", tpid, status);
#endif // DEBUG
            for(i = 0; i < RUN_MAX; i++) {
                if(pid[i] == tpid) {
#ifdef DEBUG
                    debug("solved = %d wiat solve:%d\n", running[i], run_cnt);
#endif // DEBUG
                    pid[i] = 0;
                    running[i] = 0;
                    run_cnt--;
                    break;
                }
            }
        }
        // 如果连续两次没获取到待评测
        if(!(t = get_judge()) && !(t = get_judge())) {
            if(!run_cnt) {
                if(STOP) return ; // 如果没事且要退出就退出
            } else { // 回收子进程
                while((tpid = waitpid(-1, &status, WNOHANG)) > 0) {
#ifdef DEBUG
                    debug("pid = %d status:%d\n", tpid, status);
#endif // DEBUG
                    for (i = 0; i < RUN_MAX; i++) {
                        if (pid[i] == tpid) {
                            run_cnt--;
                            pid[i] = 0;
                            running[i] = 0;
                            break;
                        }
                    }
#ifdef DEBUG
                    debug("solved = %d wiat solve:%d\n", tpid, run_cnt);
#endif
                }
            }
            sleep(2);
#ifdef DEBUG
            debug("run_cnt:%d\n",  run_cnt);
#endif
        } else {
            tpid = fork();
            if(tpid > 0) {
                run_cnt++;
                pid[i] = tpid;
                running[i] = t;
            } else if(tpid == 0) {
#ifdef DEBUG
                debug("begin run = %d in room %d\n", t, i);
#endif // DEBUG
                run(t, i);
                exit(0);
            }
        }
    }
}

int main(int argc, char** argv) {
    // 创建运行目录
    set_file();
    // 一台服务器只允许一个judge进程
    chk_running();
#ifndef DEBUG
    daemon();
    //http://www.cnblogs.com/makefile/p/3751390.html
    signal(SIGQUIT, stop_judge);
    signal(SIGINT, stop_judge);
#endif // DEBUG
    start_judge();
    execcmd("umount %s/usr %s/bin %s/lib %s/lib64", RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH);
    if(conn != NULL) mysql_close(conn);
    conn = NULL;
    warning("judge exit!\n");
    return 0;
}
