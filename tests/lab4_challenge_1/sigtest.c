#include <lib.h>

sigset_t set2;
void print(sigset_t s){
    debugf("%08x%08x", s.sig[1], s.sig[0]);
}
int main(int argc, char **argv) {
    sigset_t set;
    set.sig[0] = 529;
    set.sig[1] = 626;
    debugf("Sigset initial value = ");
    print(set);
    debugf(" \n\n");
    
    sigfillset(&set);
    debugf("After sigfillset, sigset value = ");
    print(set);
    debugf(" \n\n");    

    sigemptyset(&set);
    debugf("After sigemptyset, sigset value = ");
    print(set);
    debugf(" \n\n");

    sigaddset(&set, 2);
    sigaddset(&set, 33);
    debugf("After sigaddset 2 and 33, sigset value = ");
    print(set);
    debugf(" \n\n");

    debugf("sigismember(&set, 33) = %d\n", sigismember(&set, 33));
    debugf("sigismember(&set, 2) = %d\n", sigismember(&set, 2));
    debugf("sigismember(&set, 15) = %d\n\n", sigismember(&set, 15));

    sigdelset(&set, 2);
    debugf("After sigdelset 2, sigset value = ");
    print(set);
    debugf(" \n\n");

    sigdelset(&set, 33);
    debugf("After sigdelset 33, sigset value = ");
    print(set);
    debugf(" \n\n");
    return 0;
}
