#include <lib.h>

int global = 0;
int *test = NULL;
int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
void handler(int num) {
    debugf("Reach handler, now the signum is %d!\n", num);
    global++;
}

void sgv_handler(int num) {
    debugf("Signal appear!, signum is %d\n", num);
    test = &a[0];
    global++;
    debugf("test = %d.\n", *test);
}

int main() {
    sigset_t set;
    sigemptyset(&set);
    struct sigaction sig, oldsig;
    sig.sa_handler = handler;
    sig.sa_mask = set;
    panic_on(sigaction(2, &sig, NULL));         //注册2号信号的处理函数

    sig.sa_handler = sgv_handler;
    panic_on(sigaction(8, &sig, NULL));        //注册8号信号处理函数
    
    sigaddset(&set, 2);                         //屏蔽2和8号信号
    sigaddset(&set, 8);
    panic_on(sigprocmask(0, &set, NULL));
    kill(0, 2);                                 //发生2号信号
    kill(0, 8);                                //发生8号信号
    int ans = 0;
    for (int i = 0; i < 10000000; i++) {
        ans += i;
    }

    debugf("\npre ans is %d\n\n", ans);
    panic_on(sigprocmask(1, &set, NULL));       //解除2和8信号屏蔽
    debugf("\nafter ans is %d\n", ans);
    *test = 10;
    debugf("now test is %d\n\n", *test);

    panic_on(sigaction(2, &sig, &oldsig));      //交换两个处理函数
    panic_on(sigaction(8, &oldsig, &sig));
    panic_on(sigaction(2, &sig, NULL));
    kill(0, 2);
    kill(0, 8);

    debugf("\nglobal = %d.\n", global);
    return 0;
}
