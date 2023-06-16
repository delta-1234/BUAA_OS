#include <lib.h>

int global = 0;
void handler(int num) {
    debugf("Reach handler, now the signum is %d!\n", num);
    global++;
}

int main() {
    debugf("Big data test begin:\n");
    sigset_t set;
    sigemptyset(&set);
    struct sigaction sig;
    sig.sa_handler = handler;
    sig.sa_mask = set;
    panic_on(sigaction(2, &sig, NULL));
    panic_on(sigaction(8, &sig, NULL));

    sigaddset(&set, 2);
    sigaddset(&set, 8);
    panic_on(sigprocmask(0, &set, NULL)); //屏蔽2和8信号
    int ans = 0;
    for (int i = 0; i < 1000; i++) { //连续发送多个信号，压力测试
        kill(0, 2);
        kill(0, 8);
        ans += i;
    }

    panic_on(sigprocmask(1, &set, NULL)); //解除屏蔽
    debugf("global = %d.\n", global);
    if (global == 2000)
    debugf("Test pass!\n");
    return 0;
}
      
