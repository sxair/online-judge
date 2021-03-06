#include "soj-inc/support.h"
#include <fcntl.h>

int judge_server_id;
extern MYSQL *conn;
char run_path_fix[35];

void umount_all() {
    execcmd("umount %s/usr %s/bin %s/lib %s/etc %s/proc", RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH);
#ifdef __x86_64__
    execcmd("umount %s/lib64", RUN_PATH);
#endif // __x86_64__
}

/**
* create file for judge
*/
void set_file() {
    //以防万一，先取消所有挂载！
    umount_all();
    //建立原始文件夹
    if(execcmd("mkdir -p %s %s %s %s %s", HOME_PATH, LOG_PATH, RUN_PATH, PID_PATH, DATA_PATH)) {
        warning("file create failed");
        exit(0);
    }
    // 如果umount失败
    if(execcmd("ls %s/bin|wc -l", RUN_PATH)) {
        warning("umount失败，请用root账户运行程序\n");
        exit(0);
    }
    //删除run内内容 !必需umount后才能删除
    execcmd("rm -rf %s", RUN_PATH);

    //挂载文件
    execcmd("mkdir -p %s/usr %s/bin %s/lib %s/etc %s/proc", RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH, RUN_PATH);
    // remember to umount!
#ifdef __x86_64__
    execcmd("mkdir -p %s/lib64",RUN_PATH);
    execcmd("mount --bind /lib64 %s/lib64",RUN_PATH);
#endif // __x86_64__
    execcmd("mount --bind /usr %s/usr",RUN_PATH);
    execcmd("mount --bind /bin %s/bin",RUN_PATH);
    execcmd("mount --bind /lib %s/lib",RUN_PATH);
    execcmd("mount --bind /etc %s/etc",RUN_PATH);
    execcmd("mount --bind /proc %s/proc",RUN_PATH);

    umask(0077); // 只有自己有权限处理自己的文件
    for (int i = 0; i < MAX_RUN; i++) {
        execcmd("mkdir -p %s/run%d", RUN_PATH, i);
        execcmd("useradd judge%d -u %d", i, JUDGER_BASE_ID + i);
        execcmd("chown judge%d:judge%d %s/run%d", i, i, RUN_PATH, i);
    }
}

/**
* check if have another judger
*/
void chk_running() {
    char buf[64];
    sprintf(buf, "%s/judge.pid", PID_PATH);
    // O:读写。不存在则新建 创建时：S（受unmask限制）:用户读写，组可读
    int fd = open(buf, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP);
    if (fd < 0) {
        printf("oj already running.\n");
        exit(1);
    }
    struct flock fl;
    fl.l_len = 0;
    fl.l_start = 0;
    fl.l_type = F_WRLCK;//lock for write
    fl.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        printf("lock %s failed:%s",buf,strerror(errno));
        exit(1);
    }
    if(!ftruncate(fd, 0)) { // clear content
        sprintf(buf, "%d", getpid());
        int t = write(fd, buf, strlen(buf));
    }
    //close(fd); 不可关闭
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
    chk_running(); //重新锁一遍
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

bool STOP = false;
void stop_judge(int s) {
    STOP = true;
}

void run(unsigned long long judge_id, int run_id) {
#ifdef DEBUG
    printf("begin run judge_id= %llu in room %d\n", judge_id, run_id);
#endif // DEBUG
    char judgeid[68], run_room[37];
    sprintf(judgeid, "%llu",judge_id);
    sprintf(run_room, "%s%d", run_path_fix, run_id);
#ifdef TEST
    sleep(2);
#else
    if (execl("./run-client", "./run-client", judgeid, run_room, (char *)NULL) == -1) {
        warning("error:run程序无法运行\n");
        exit(0);
    }
#endif // TEST
}

int queue_top, queue_last, run_cnt;
unsigned long long judge_queue[MAX_RUN << 1 | 1];
pid_t pid[MAX_RUN + 1], tpid;

bool set_judges() {
    queue_last = queue_top = 0;
    if(conn == NULL && !connect_mysql()) {
        return false;
    }

    execsql("UPDATE judges set running=%d WHERE running=0 LIMIT %d", judge_server_id, MAX_RUN << 1);
    execsql("SELECT status_id,judge_for FROM judges WHERE running=%d LIMIT %d", judge_server_id, MAX_RUN << 1);
    // http://www.jb51.net/article/19661.htm
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
    execsql("UPDATE judges set running=%d WHERE running=%d", judge_server_id + MAX_JUDGE_ID, judge_server_id);
    printf("get %d judges\n", queue_top);
    return true;
}

/**
* 获取评测id
*/
unsigned long long get_judge() {
    // 先从队列中获取。初始都为0  获取不到则从数据库中拉取
    if(queue_last < queue_top || (set_judges() && queue_last < queue_top)) {
        return judge_queue[queue_last++];
    }
    return 0;
}

/**
* 把子进程id从pid中删除,并返回run room id
*/
int solve_pid(pid_t tpid) {
    for(int i = 0; i < MAX_RUN; i++) {
        if(pid[i] == tpid) {
            pid[i] = 0;
#ifdef DEBUG
            printf("solve pid %d\n", tpid);
#endif // DEBUG
            run_cnt--;
            return i;
        }
    }
    return -1;
}

/**
* 回收所有子进程
*/
void wait_all() {
    if(run_cnt) {
        while((tpid = waitpid(-1, NULL, WNOHANG)) > 0) {
            solve_pid(tpid);
        }
    }
}

void start_judge() {
    int i;
    unsigned long long t;
    //把之前未解决的重新加入队列
    execsql("update judges set running = 0 where running = %d", judge_server_id + MAX_JUDGE_ID);
    while(1) {
        // 获取空闲位置
        if(run_cnt < MAX_RUN) {
            for(i = 0; pid[i]; i++) ;
        } else {
            //等待一个进程结束。并在队列中删除
            i = solve_pid(waitpid(-1, NULL, 0));
        }
        // 如果需要停止。则回收所有子进程
        if(STOP) {
            wait_all();
            return;
        }
        // 如果没获取到待评测
        if(!(t = get_judge())) {
            wait_all();
            sleep(2);
        } else {
            tpid = fork();
            if(tpid > 0) {
                run_cnt++;
                pid[i] = tpid;
            } else if(tpid == 0) {
                run(t, i);
                exit(0);
            }
        }
    }
}

int main(int argc, char** argv) {
    // 一台服务器只允许一个judge进程
    chk_running();
    // 创建运行目录
    set_file();
    if(argc > 1) {
        judge_server_id = atoi(argv[1]);
    } else judge_server_id = JUDGE_ID;
#ifndef DEBUG
    daemon();
    //http://www.cnblogs.com/makefile/p/3751390.html
    //https://www.cnblogs.com/hanyifeng/p/6728151.html
    signal(SIGQUIT, stop_judge);
    signal(SIGINT, stop_judge);
    signal(SIGTERM, stop_judge);
#endif // DEBUG
    //begin
    start_judge();
    // end
    umount_all();
    close_mysql();
    return 0;
}
