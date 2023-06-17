#include <lib.h>

int global = 0;
int global2 = 0;
void handler(int num) {
    global++;
    debugf("Reach handler, now glboal = %d\n", global);
    if (global < 10) {
        kill(0,32); //被屏蔽
        kill(0, 64); //递归
    }
    debugf("Leave handler, now sig=%d\n", num);
}

void handler2(int num) {
    global2++;
    debugf("Now can run sig=%d\n", num);
}

int main() {
    struct sigaction sig;
    sig.sa_handler = handler;
    sigemptyset(&sig.sa_mask);
    sigaddset(&sig.sa_mask, 32); //处理64号信号时屏蔽32号信号
    panic_on(sigaction(64, &sig, NULL));

    sigemptyset(&sig.sa_mask);
    sig.sa_handler = handler2;
    panic_on(sigaction(32, &sig, NULL));

    debugf("Signal send...\n");
    kill(0, 64);
    if (global == 10 && global2 == 9) 
        debugf("Test pass!\n");
    return 0;
}

