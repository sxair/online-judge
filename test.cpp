#include<bits/stdc++.h>
#include<wait.h>
#include<unistd.h>
#include <sys/resource.h>
using namespace std;

void dfs(){
    int a;
    dfs();
}
int a[123];

int main() {
    pid_t pid = fork();
    if(pid==0) {
        int a=1;
        dfs();
    } else {
        int status, sig;
        waitpid(pid, &status, 0);
        if(WIFSTOPPED(status)) {
            sig = WSTOPSIG(status);
            printf("stop:%d\n", sig);
        }
        if (WIFSIGNALED(status)){
            sig = WTERMSIG(status);
            printf("sigend:%d %s\n", sig, strsignal(sig));
        }
    }
}
