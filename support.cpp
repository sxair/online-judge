#include "support.h"

void debug(const char *fmt, ...) {
#ifdef DEBUG
    char buf[LOG_MAX_BUFF];
    va_create(buf);
    printf("%s", buf);
#endif
}

void write_log(const char *fmt, ...) {
    char buf[LOG_MAX_BUFF];
    #ifdef DEBUG
            debug("write log:%s\n", buf);
        #endif // DEBUG
    time_t sec = time(NULL);
    struct tm *t = localtime(&sec);
    sprintf(buf, "%s/%djudge%d.log", LOG_PATH, t->tm_year + 1900, t->tm_mon + 1);
    FILE *fp = fopen(buf, "a+");
    if (fp == NULL) {
        syslog(LOG_ERR, "write log error,path:%s\n maybe log file is not exist\n", buf);
        #ifdef DEBUG
            debug("write log error,path:%s\n maybe log file is not exist\n", buf);
        #endif // DEBUG
        return ;
    }
    va_create(buf);
    fprintf(fp, "%d %02d:%02d:%02d %s", t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, buf);
    fclose(fp);
}

void warning(const char *fmt, ...) {
    char buf[LOG_MAX_BUFF];
    va_create(buf);
#ifdef DEBUG
    debug("%s", buf);
#endif // DEBUG
    write_log("%s", buf);
}

int execcmd(const char *fmt, ...) {
    char cmd[256];
    va_create(cmd);
#ifdef DEBUG
    debug("exec %s\n",cmd);
#endif
    return system(cmd);
}

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
        #ifdef DEBUG
            debug("mysql connetc success\n");
        #endif
        mysql_real_query(conn, "set names utf8mb4", 18);
    }
    return true;
}

bool execsql(const char *fmt, ...) {
    char sql[LOG_MAX_BUFF];
    va_create(sql);
#ifdef DEBUG
    debug("query %s\n", sql);
#endif
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
