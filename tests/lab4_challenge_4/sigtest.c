#include <lib.h>

int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int *test = NULL;
void null_handler(int num) {
	debugf("Segment fault appear!\n");
	test = &a[0];
	debugf("test = %d.\n", *test);
	debugf("env %d exit\n", syscall_getenvid);
	exit();
}

void kern_stop_handler(int num) {
	debugf("signal %d can't be blocked\n", num);
	debugf("env %d exit\n", syscall_getenvid);
	exit();
}

void user_stop_handler(int num) {
	debugf("signal %d can be blocked\n", num);
}

int main(int argc, char **argv) {
	sigset_t set;
	sigemptyset(&set);
	struct sigaction sig;
	sig.sa_handler = null_handler;
	sig.sa_mask = set;
	panic_on(sigaction(11, &sig, NULL)); // SIGSEGV

	sig.sa_handler = kern_stop_handler;
	panic_on(sigaction(9, &sig, NULL)); // SIGKILL

	sig.sa_handler = user_stop_handler;
	panic_on(sigaction(15, &sig, NULL)); // SIGTERM

    sigaddset(&set, 11);
    sigaddset(&set, 9);
    sigaddset(&set, 15);
    sigprocmask(0, &set, NULL);

	if (fork()) {
        debugf("null test env is %d\n", syscall_getenvid);
        sigdelset(&set, 11);
        sigprocmask(2, &set, NULL);
        *test = 10;
	} else {
        if (fork()) {
            debugf("SIGKILL test env is %d\n", syscall_getenvid);
            kill(0, 9);
        } else {
            kill(0, 15);
            debugf("SIGTERM test env is %d\n", syscall_getenvid);
            sigdelset(&set, 15);
            sigprocmask(2, &set, NULL);
            debugf("can't stop now\n");
        }
    }
	return 0;
}