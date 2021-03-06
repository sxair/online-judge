#include "soj-inc/support.h"

char log_buf[MAX_BUFF << 1];

long get_file_size(const char *file) {
    struct stat f_stat;
    stat(file, &f_stat);
    return f_stat.st_size;
}

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

void warning(const char *fmt, ...) {
    time_t sec = time(NULL);
    struct tm *t = localtime(&sec);
    char path_buf[128];
    va_create(log_buf);
    sprintf(path_buf, "%s/%djudge%d.log", LOG_PATH, t->tm_year + 1900, t->tm_mon + 1);
    FILE *fp = fopen(path_buf, "a+");
    if (fp == NULL) {
        syslog(LOG_ERR, "write log error,path:%s\ncontent:%s\n maybe log file is not exist\n", path_buf, log_buf);
        #ifdef DEBUG
            printf("write log error,path:%s\ncontent:%s\n maybe log file is not exist\n", path_buf, log_buf);
        #endif // DEBUG
        return ;
    }
    #ifdef DEBUG
        printf("log:%s\n",log_buf);
    #endif // DEBUG
    fprintf(fp, "%d %02d:%02d:%02d %s", t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, log_buf);
    fclose(fp);
}

int execcmd(const char *fmt, ...) {
    char cmd[256];
    va_create(cmd);
#ifdef DEBUG
    printf("exec %s\n",cmd);
#endif
    return system(cmd);
}

/*-------------------------------db---------------------------*/
MYSQL *conn;

bool connect_mysql() {
    if(conn == NULL) {
        conn = mysql_init(NULL); //用来分配或者初始化一个MYSQL对象，用于连接mysql服务端
        const char timeout = 30; // 连接失败重新连接时间
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PWD, DB_NAME, DB_PORT, 0, 0)) {
            warning("connect mysql error:%s\n",mysql_error(conn));
            mysql_close(conn);
            conn = NULL;
            return false;
        }
        if (mysql_real_query(conn, "set names utf8mb4", 17)) {
            warning("连接数据库失败\n");
            if(conn != NULL)
                mysql_close(conn);
            conn = NULL;
            exit(0);
        }
        #ifdef DEBUG
            printf("mysql connetc success\n");
        #endif // DEBUG
    }
    return true;
}
char sql[MAX_BUFF << 1];
bool execsql(const char *fmt, ...) {
    va_create(sql);
#ifdef DEBUG
    printf("execsql %s\n",sql);
#endif // DEBUG
    if (conn == NULL && !connect_mysql()) {
        warning("exec sql error:%s\nmysql can not connect\n", sql);
        return false;
    }
    //mysql_real_query 执行失败则返回>0
    if (mysql_real_query(conn, sql, strlen(sql))) {
        warning("exec sql error:%s\nmysql_error: %s\n", sql, mysql_error(conn));
        if(conn != NULL)
            mysql_close(conn);
        conn = NULL;
        return false;
    }
    return true;
}

void close_mysql() {
    if(conn != NULL) mysql_close(conn);
    conn = NULL;
}
