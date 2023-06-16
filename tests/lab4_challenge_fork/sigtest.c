#include <lib.h>
int cnt = 0;
int father;
void handler(int num) {
	cnt++;
	if (cnt > 5) {
		return;
	}
	if (syscall_getenvid() == father) {
		debugf("father get signal=%d, cnt=%d\n", num, cnt);
	} else {
        debugf("child get signal=%d, cnt=%d\n", num, cnt);
    }
}

int main() {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_handler = handler;
	panic_on(sigaction(15, &act, NULL));
    panic_on(sigaction(25, &act, NULL));
	father = syscall_getenvid();
	int r = fork();
	if (r != 0) {
		sigset_t set;
		sigemptyset(&set);
		for (int i = 0; i < 5; i++) {
			kill(r, 25); //向子进程发送25信号
		}
		while (cnt != 5); //等待子进程发送完信号
		debugf("father test passed!\n");
	} else {
		while (cnt != 5); //等待父进程发送完信号
		debugf("child test passed!\n");
		for (int i = 0; i < 5; i++) {
			kill(father, 15); //向子进程发送15信号
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
