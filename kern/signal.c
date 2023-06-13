#include <env.h>
#include <pmap.h>
#include <signal.h>
#include <env.h>

extern struct Env *curenv;

int sigismember(sigset_t *set, int signum) {
	u_int temp;
	if (signum > 64) {
		return 0;
	}
	if (signum <= 32) {
		temp = 0x1 << (signum - 1);
		return set->sig[0] & temp;
	} else {
		temp = 0x1 << (signum - 33);
		return set->sig[1] & temp;
	}
}

void do_signal() {
	struct signal_node *temp, *pre_temp;
	if (curenv->signal_list.len != 0) {
		temp = curenv->signal_list.head;
		while (temp != NULL) {
			if (sigismember(&curenv->signal_mask, temp->signal)) {
				break;
			} else {
                pre_temp = temp;
                temp = temp->next;
            }
		}
		if (temp != NULL) {
			printk("%d\n", temp->signal);
			struct Trapframe *tf = &curenv->env_tf;
			struct Trapframe tmp_tf = *tf;
			if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
				tf->regs[29] = UXSTACKTOP;
			}
			tf->regs[29] -= sizeof(struct Trapframe);
			*(struct Trapframe *)tf->regs[29] = tmp_tf;
			if (curenv->sigaction_list[temp->signal-1].sa_handler) {
				tf->regs[4] = temp->signal;
				tf->regs[31] = tf->regs[31];
				tf->cp0_epc = (u_int) curenv->sigaction_list[temp->signal-1].sa_handler;
				printk("addr %d\n", tf->cp0_epc);
			} else {
				env_destroy(curenv);
			}
            if (temp != curenv->signal_list.head) {
                pre_temp->next = temp->next;
            } else {
                curenv->signal_list.head = temp->next;
            }
			printk("len %d\n", curenv->signal_list.len);
            curenv->signal_list.len--;
			env_pop_tf(&(curenv->env_tf), curenv->env_asid);
		}
	}
}
