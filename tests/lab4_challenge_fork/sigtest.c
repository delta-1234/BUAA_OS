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

// #include <lib.h>

// int global = 0;
// void handler(int num) {
//     debugf("Reach handler, now the signum is %d!\n", num);
//     global = 1;
// }

// #define TEST_NUM 2
// int main(int argc, char **argv) {
//     sigset_t set;
//     sigemptyset(&set);
//     struct sigaction sig;
//     sig.sa_handler = handler;
//     sig.sa_mask = set;
//     panic_on(sigaction(TEST_NUM, &sig, NULL));
//     sigaddset(&set, TEST_NUM);
//     panic_on(sigprocmask(0, &set, NULL));
//     kill(0, TEST_NUM);
//     int ans = 0;
//     for (int i = 0; i < 10000000; i++) {
//         ans += i;
//     }
//     panic_on(sigprocmask(1, &set, NULL));
//     debugf("global = %d.\n", global);
//     return 0;
// }

// #include <lib.h>

// int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
// int *test = NULL;
// void sgv_handler(int num) {
//     debugf("Segment fault appear!\n");
//     test = &a[0];
//     debugf("test = %d.\n", *test);
//     exit();
// }

// int main(int argc, char **argv) {
//     sigset_t set;
//     sigemptyset(&set);
//     struct sigaction sig;
//     sig.sa_handler = sgv_handler;
//     sig.sa_mask = set;
//     panic_on(sigaction(11, &sig, NULL));
//     *test = 10;
//     debugf("test = %d.\n", *test);
//     return 0;
// }

// #include <lib.h>

// sigset_t set2;

// int main(int argc, char **argv) {
//     sigset_t set;
//     sigemptyset(&set);
//     sigaddset(&set, 1);
//     sigaddset(&set, 2);
//     panic_on(sigprocmask(0, &set, NULL));
//     sigdelset(&set, 2);
//     int ret = fork();
//     if (ret != 0) {
//         panic_on(sigprocmask(0, &set2, &set));
//         debugf("Father: %d.\n", sigismember(&set, 2));
//     } else {
//         debugf("Child: %d.\n", sigismember(&set, 2));
//     }
//     return 0;
// }

