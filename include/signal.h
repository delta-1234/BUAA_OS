#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#define SIGKILL 9
#define SIGSEGV 11
#define SIGTERM 15

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

struct signal_node {
    int signal;
    struct Trapframe oldTf;
    struct signal_node *next;
};

struct signal_quene {
    int len;
    struct signal_node *head;
};

typedef struct {
    u_int sig[2]; //最多64个信号
} sigset_t;

struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
};

void signal_finish();
int send_signal(u_int envid, int sig);
void do_signal(struct Trapframe *tf);

#endif