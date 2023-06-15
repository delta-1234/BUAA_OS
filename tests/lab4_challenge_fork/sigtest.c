#include <lib.h> 
int cnt=0;
void handler(int num){
    cnt++;
    if(cnt>5){
        return ;
    }
    debugf("cnt:%d HANDLER:%x %d\n",cnt,syscall_getenvid(),num);
}

int main(int argc, char **argv) {
    //先注册了一个信号
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler=handler;
    sigaction(10,&act,NULL);
    int father = syscall_getenvid();
    int ret = fork();
    debugf("father:%d child:%d\n",syscall_getenvid(),ret);
    if (ret != 0) {
        sigset_t set;
        sigemptyset(&set);
        for(int i=0;i<5;i++){
            kill(ret,10);
        }
        while(cnt!=5);
        debugf("Father passed!\n");
    } else {
        while(cnt!=5);
        debugf("Child passed!\n");
        for(int i=0;i<5;i++){
            kill(father,10);
        }
    }
    return 0;
}       
