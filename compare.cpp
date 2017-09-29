#include "soj-inc/compare.h"

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

long get_file_size(const char *file) {
    struct stat f_stat;
    stat(file, &f_stat);
    return f_stat.st_size;
}

bool ispe(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

int check_ans(const char *pro,const char *user) {
    // if(spj) return check_ans_spj(pro, user);
    FILE *fp1, *fp2;
    int flag = OJ_AC,i,j, ii, jj;
#ifdef DEBUG
    debug("file:%s %s\n",pro,user);
#endif // DEBUG
    if((fp1= fopen(pro, "rb")) == NULL) {
        return OJ_AC;
    }
    if((fp2= fopen(user, "rb")) == NULL) {
        return OJ_WA;
    }
    int la = get_file_size(pro);
    int lb = get_file_size(user);
    char *p = (char*)malloc(sizeof(char)*la + 2);
    char *s = (char*)malloc(sizeof(char)*lb + 2);
    if(fread(p, 1, la + 1, fp1));
    if(fread(s, 1, lb + 1, fp2));
    for(i=0, j=0; i<la&&j<lb; i++,j++) {
        if(p[i] == 13) i++;
        if(p[i]!=s[j]) {
            if(!ispe(p[i])&&!ispe(s[j])) {
                free(p);
                free(s);
                return OJ_WA;
            }
            break;
        }
    }
    if(i==la&&j==lb) {
        free(p);
        free(s);
        return OJ_AC;
    }
    for(ii=0; i<la; i++) {
        while(ispe(p[i])) ++i;
        if(i==la) break;
        p[ii++] = p[i];
    }
    p[ii] = '\0';
    for(jj=0; j<lb; j++) {
        while(ispe(s[j])) ++j;
        if(j==lb) break;
        s[jj++] = s[j];
    }
    s[jj] = '\0';
    if(!strcmp(s,p)) {
        flag = OJ_PE;
    } else {
        flag = OJ_WA;
    }
    free(p);
    free(s);
    return flag;
}
