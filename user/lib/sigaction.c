#include <sigaction.h>
#include <lib.h>

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    return syscall_sigaction(signum, act, oldact);
}


int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    return syscall_sigprocmask(how, set, oldset);
}

// 清空信号集，将所有位都设置为 0
void sigemptyset(sigset_t *set) {
    set->sig[0] = 0;
    set->sig[1] = 0;
}

// 设置信号集，即将所有位都设置为 1
void sigfillset(sigset_t *set) {
    set->sig[0] = 0xffffffff;
    set->sig[1] = 0xffffffff;
}

// 向信号集中添加一个信号，即将指定信号的位设置为 1
void sigaddset(sigset_t *set, int signum) {
    u_int temp;
    if (signum > 64) {
        return;
    }
    if (signum <= 32) {
        temp = 0x1 << (signum - 1);
        set->sig[0] |= temp;
    } else {
        temp = 0x1 << (signum - 33);
        set->sig[1] |= temp;
    }
}

// 从信号集中删除一个信号，即将指定信号的位设置为 0
void sigdelset(sigset_t *set, int signum) {
    u_int temp;
    if (signum > 64) {
        return;
    }
    if (signum <= 32) {
        temp = 0x1 << (signum - 1);
        temp = ~temp;
        set->sig[0] &= temp;
    } else {
        temp = 0x1 << (signum - 33);
        temp = ~temp;
        set->sig[1] &= temp;
    }
}

// 检查一个信号是否在信号集中，如果在则返回 1，否则返回 0
int sigismember(const sigset_t *set, int signum) {
    u_int temp;
    if (signum > 64) {
        return 0;
    }
    if (signum <= 32) {
        temp = 0x1 << (signum - 1);
        if (set->sig[0] & temp) {
            return 1;
        } else {
            return 0;
        }
    } else {
        temp = 0x1 << (signum - 33);
        if (set->sig[1] & temp) {
            return 1;
        } else {
            return 0;
        }
    }
}

int kill(u_int envid, int sig) {
    return syscall_send_signal(envid, sig);
}