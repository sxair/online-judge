#include "lang.h"

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

bool ispe(char c){
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

int check_ans(const char *pro,const char *user) {
    // if(spj) return check_ans_spj(pro, user);
    FILE *fp1, *fp2;
    int flag = OJ_AC,i,j, ii, jj;
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
    printf("la:%d lb:%d\n",la,lb);
    fread(p, 1, la + 1, fp1);
    fread(s, 1, lb + 1, fp2);
    for(i=0, j=0;i<la&&j<lb;i++,j++){
        if(p[i] == 13) i++;
        if(p[i]!=s[j]) {
            if(!ispe(p[i])&&!ispe(s[j])){
                free(p);
                free(s);
                return OJ_WA;
            }
            break;
        }
    }
    if(i==la&&j==lb){
        free(p);
        free(s);
        return OJ_AC;
    }
    for(ii=0;i<la;i++){
        while(ispe(p[i])) ++i;
        if(i==la) break;
        p[ii++] = p[i];
    }
    p[ii] = '\0';
    for(jj=0;j<lb;j++){
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

int run_test(int (*lang_run)(const char *)) {
    int f = OJ_AC, i, fal = OJ_AC;
    char buf[128],tbuf[128];
    for(i = 1; i <= judge_cnt; i++) {
        sprintf(buf, "%s/%d/pro%d_test%d.in", DATA_PATH, problem_id, problem_id, i);
        f = lang_run(buf);
#ifdef DEBUG
        debug("running test %d , get status:%d\n",i,f);
#endif // DEBUG
        if(f != OJ_AC) {
            fal = f;
            break;
        }
        sprintf(buf, "%s/%d/pro%d_test%d.out", DATA_PATH, problem_id, problem_id, i);
        sprintf(tbuf, "%ld.out", name);
        f = check_ans(buf, tbuf);
        if(f == OJ_WA) {
            fal = OJ_WA;
            break;
        }
        if(f == OJ_PE) {
            fal = OJ_PE;
            break;
        }
    }
    if(fal == OJ_AC) return OJ_AC;
    return fal * OJ_TEST_MAX + i;
}

int get_proc_status(int pid, const char *type) {
    FILE * fp;
    char path[64], buf[1024];
    int ans = 0, l = strlen(type);
    sprintf(path, "/proc/%d/status", pid);
    if((fp = fopen(path, "r")) != NULL) {
        while (fgets(buf, 1023, fp)) {
            if (strncmp(buf, type, l - 1) == 0) {
                sscanf(buf + l, "%d", &ans);
                fclose(fp);
                break;
            }
        }
    }
    return ans;
}
