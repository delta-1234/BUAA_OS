#include <lib.h>

int global = 0;
void handler(int num) {
    global++;
    debugf("Reach handler, now glboal = %d\n", global);
    if (global < 10) {
        kill(0, 64); //递归
    }
    debugf("Leave handler\n");
}

int main() {
    struct sigaction sig;
    sig.sa_handler = handler;
    sigemptyset(&sig.sa_mask);
    panic_on(sigaction(64, &sig, NULL));
    debugf("signal send...\n");
    kill(0, 64);
    if (global == 10) 
        debugf("Test pass!\n");
    return 0;
}

