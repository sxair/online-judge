#ifndef SUPPORT_H_INCLUDED
#define SUPPORT_H_INCLUDED

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <mysql/mysql.h>
#include <sys/resource.h>
#include "config.h"

#define va_create(buf) va_list ap; va_start(ap,fmt); vsprintf(buf, fmt, ap); va_end(ap);

long get_file_size(const char *file);
void write_file(const char *cont, const char *fmt, ...);

// log
void write_log(const char *, ...) ;
void warning(const char *, ...) ;

int execcmd(const char *, ...) ;

//db
bool connect_mysql() ;
bool execsql(const char *, ...) ;
void close_mysql();

#endif // SUPPORT_H_INCLUDED
