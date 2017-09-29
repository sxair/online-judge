#include "soj-inc/support.h"
#include <fcntl.h>

extern MYSQL *conn;

void set_file() {
    execcmd("umount %s/usr %s/bin %s/lib %s/etc %s/proc", RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH);
#ifdef __x86_64__
    execcmd("umount %s/lib64", RUN_PATH);
    execcmd("mkdir -p %s/lib64",RUN_PATH);
    execcmd("mount --bind /lib64 %s/lib64",RUN_PATH);
#endif // __x86_64__
    execcmd("mkdir -p %s %s %s %s %s", HOME_PATH, LOG_PATH, RUN_PATH, PID_PATH, DATA_PATH);
    for (int i = 0; i < MAX_RUN; i++) {
        execcmd("mkdir -p %s%d", RUN_ROOM_PREFIX, i);
        execcmd("chown %s:%s %s%d", POORUSER, POORUSER, RUN_ROOM_PREFIX, i);
    }
    execcmd("mkdir -p %s/usr %s/bin %s/lib %s/etc %s/proc", RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH);
    // remember to umount!
    execcmd("mount --bind /usr %s/usr",RUN_PATH);
    execcmd("mount --bind /bin %s/bin",RUN_PATH);
    execcmd("mount --bind /lib %s/lib",RUN_PATH);
    execcmd("mount --bind /etc %s/etc",RUN_PATH);
    execcmd("mount --bind /proc %s/proc",RUN_PATH);
}

void chk_running() {
    char buf[64];
    sprintf(buf, "%s/judge.pid", PID_PATH);
    // O:读写。不存在则新建 创建时：S（受unmask限制）:用户读写，组可读
    int fd = open(buf, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP);
    if (fd < 0) {
        warning("oj already running.\n");
        exit(1);
    }
    struct flock fl;
    fl.l_len = 0;
    fl.l_start = 0;
    fl.l_type = F_WRLCK;//lock for write
    fl.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        warning("lock %s failed:%s",buf,strerror(errno));
        exit(1);
    }
    if(!ftruncate(fd, 0)) { // clear content
        sprintf(buf, "%d", getpid());
        int t = write(fd, buf, strlen(buf));
    }
    close(fd);
}

void daemon(void) {
    pid_t pid;
    while((pid = fork())< 0) {
        printf("daemon ceate faild\n");
        exit(0);
    }
    if (pid != 0) exit(0); // father eixt
    printf("daemon ceate success\n judging now\n");
    setsid(); // give son a id
    printf("judge server pid: %d\n",getpid());
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
}

void run(unsigned long long judge_id, int run_id) {
    char judgeid[68], runid[68];
    sprintf(judgeid, "%llu",judge_id);
    sprintf(runid, "%d", run_id);
    if (execl("./run-client", "./run-client", judgeid, runid, (char *)NULL) == -1) {
        warning("error:run程序无法运行\n");
    }
}

int queue_top, queue_last, run_cnt;
unsigned long long judge_queue[MAX_RUN << 1 | 1];
pid_t pid[MAX_RUN + 1], tpid;

void set_judges() {
    if(!connect_mysql()) {
        return ;
    }
    queue_top = 0;
    execsql("select status_id,judge_for from judges where running = 0 limit %d", MAX_RUN << 1);
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
}

unsigned long long get_judge() {
    if(queue_last < queue_top) {
        return judge_queue[queue_last++];
    }
    set_judges();
    if(queue_last < queue_top) {
        return judge_queue[queue_last++];
    }
    return queue_last = 0;
}

inline void solve_pid(pid_t tpid) {
    for(int i = 0; i < MAX_RUN; i++) {
        if(pid[i] == tpid) {
            pid[i] = 0;
            run_cnt--;
            break;
        }
    }
}

void start_judge() {
    int i;
    unsigned long long t;
    execsql("update judges set running = 0 where running = 1");
    while(1) {
        // 获取空闲位置
        if(run_cnt < MAX_RUN) {
            for(i = 0; pid[i]; i++) ;
        } else {
            solve_pid(waitpid(-1, NULL, 0));
        }
        // 如果没获取到待评测
        if(!(t = get_judge())) {
            if(!run_cnt) {
                if(STOP) return ; // 如果没事且要退出就退出
            } else { // 回收子进程
                while((tpid = waitpid(-1, NULL, WNOHANG)) > 0) {
                    solve_pid(tpid);
                }
            }
            sleep(2);
        } else {
            execsql("update judges set running = 1 where status_id = %u and judge_for = %u",
                    (unsigned)(t & 0xffffffff), (unsigned)(t >> 32));
            tpid = fork();
            if(tpid > 0) {
                run_cnt++;
                pid[i] = tpid;
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
    //begin
    start_judge();
    // end
    execcmd("umount %s/usr %s/bin %s/lib %s/lib64", RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH);
    close_mysql();
    warning("judge exit!\n");
    return 0;
}
