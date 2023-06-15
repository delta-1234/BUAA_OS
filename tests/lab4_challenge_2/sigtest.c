#include <lib.h>

int global = 0;
void func(int num) {
    global++;
    debugf("Reach handler! Glboal = %d\n", global);
    if (global < 200) {
        kill(0, 4);
    }
    debugf("Leave handler\n");
}

int main() {
    debugf("Recursion test\n");
    struct sigaction mysigaction;
    mysigaction.sa_handler = func;
    sigemptyset(&mysigaction.sa_mask);
    sigaction(4, &mysigaction, NULL);
    kill(0, 4);
    return 0;
}
    
