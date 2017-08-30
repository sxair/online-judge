#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define DB_HOST  "127.0.0.1"
#define DB_NAME  "oj"
#define DB_USER  "root"
#define DB_PWD   "zx"
#define DB_PORT  3306
// add a path remeber add in judge.cpp -> set_file()
#define HOME_PATH "/home/judge"
#define LOG_PATH  "/home/judge/log" // if wrong, see /var/log/syslog
#define RUN_PATH  "/home/judge/run"
#define PID_PATH  "/home/judge/etc"
#define DATA_PATH "/home/judge/data"

#define RUN_MAX  3

#define LOG_MAX_BUFF 4096

#define POORUID  1001
#define POORUSER "judge"

#define OJ_JG 1
#define OJ_WA 2
#define OJ_CE 3
#define OJ_MLE 4
#define OJ_OLE 5
#define OJ_TLE 6
#define OJ_PE 7
#define OJ_AC 8
#define OJ_RE 10
#define OJ_OF 11
#define OJ_AS 12
#define OJ_DZ 13

#define OJ_TEST_MAX 10000
#define OJ_RUN_TIME 120

#define CONTEST_BASE 0x10000000
#define DIT_BASE     0x20000000

#endif // CONFIG_H_INCLUDED
