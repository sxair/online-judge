#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define JUDGE_ID 1 //评判机编号。用于恢复评判机失败评判队列

#define DB_HOST  "127.0.0.1"
#define DB_NAME  "soj"
#define DB_USER  "root"
#define DB_PWD   "zx"
#define DB_PORT  3306

// add a path remeber add in judge.cpp -> set_file()
#define HOME_PATH "/home/judge"
#define LOG_PATH  "/home/judge/log" // if wrong, see /var/log/syslog
#define RUN_PATH  "/home/judge/run"

#define PID_PATH  "/home/judge/etc"
#define DATA_PATH "/var/www/soj/storage/app/data"

#define MAX_RUN  10 // run-client number
#define MAX_BUFF 2048 // log buff
#define MAX_TEST 10000 // 测试数据的数量
#define MAX_RUN_TIME 60 //second

#define POORUID  1001 // 低权限用户 see /etc/passwd
#define POORUSER "judge"

#define OJ_CI 1 //Compiling
#define OJ_CE 2
#define OJ_AC 3
#define OJ_SE 4 //System Error
#define OJ_RI 5 //Running
#define OJ_WA 6
#define OJ_MLE 7
#define OJ_OLE 8
#define OJ_TLE 9
#define OJ_PE 10
#define OJ_RE 11
#define OJ_SF 12 //Segmentation fault sig:11
#define OJ_FPE 13 //Floating Point Exception sig:8
#define OJ_NASC 14 //not allow SYSTEM CALL

#endif // CONFIG_H_INCLUDED
