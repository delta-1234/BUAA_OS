#include <env.h>
#include <pmap.h>
#include <signal.h>

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

struct signal_node *free_list = NULL;

int sig_alloc(struct signal_node **new) {
	if (free_list == NULL) {
		struct Page *p;
		page_alloc(&p);
		p->pp_ref++;
		free_list = (struct signal_node *)page2kva(p);
		free_list->next = NULL;
		struct signal_node *temp;
		for (temp = free_list + 1; temp + 1 < page2kva(p) + BY2PG; temp++) {
			temp->next = free_list;
			free_list = temp;
		}
	}
	*new = free_list;
	free_list = free_list->next;
	return 0;
}

int send_signal(u_int envid, int sig) {
	if (sig > 64) {
		return -1;
	}
	struct Env *env;
	struct signal_node *new;
	if (envid == 0) {
		env = curenv;
	} else {
		int r = envid2env(envid, &env, 0);
		if (r < 0) {
			return -1;
		}
	}
	if (env->signal_list.head == NULL) {
		sig_alloc(&new);
		env->signal_list.len++;
		env->signal_list.head = new;
		new->next = NULL;
		new->signal = sig;
	} else {
		sig_alloc(&new);
		env->signal_list.len++;
		new->next = env->signal_list.head;
		new->signal = sig;
		env->signal_list.head = new;
	}
	env_run(env);
}

void signal_finish() {
	//printk("2\n");
	struct Trapframe *temp_tf = &curenv->cur_signal->oldTf;
	curenv->signal_mask = curenv->cur_signal->oldMask;
	curenv->cur_signal = curenv->cur_signal->next;
	do_signal(temp_tf);
	env_pop_tf(temp_tf, curenv->env_asid);
}

void do_signal(struct Trapframe *tf) {
	struct signal_node *temp, *pre_temp;
	int flag = 0;
	// printk("%d\n", curenv->signal_list.len);
	if (curenv->signal_list.len != 0 && !curenv->isCow) {
		temp = curenv->signal_list.head;
		while (temp != NULL) {
			if (!sigismember(&curenv->signal_mask, temp->signal)) {
				break;
			} else {
				pre_temp = temp;
				temp = temp->next;
			}
		}
		if (temp != NULL) {
			if (curenv->sigaction_list[temp->signal - 1].sa_handler) {
				// memcpy(&temp->oldTf, tf, sizeof(struct Trapframe));
				temp->oldTf = *tf;
				temp->oldMask = curenv->signal_mask;
				// curenv->signal_mask = curenv->sigaction_list[temp->signal - 1].sa_mask;
				curenv->signal_mask.sig[0] |= curenv->sigaction_list[temp->signal - 1].sa_mask.sig[0];
				curenv->signal_mask.sig[1] |= curenv->sigaction_list[temp->signal - 1].sa_mask.sig[1];
				curenv->signal_mask.sig[0] &= (~(0x1 << (SIGKILL - 1))); //不允许屏蔽9号中断
				flag = 1;
				tf->regs[4] = temp->signal;
				tf->regs[31] = curenv->signal_return;
				// printk("%d\n", curenv->signal_return);
				tf->cp0_epc =
				    (u_int)curenv->sigaction_list[temp->signal - 1].sa_handler;
			} else if (temp->signal == SIGKILL || temp->signal == SIGSEGV ||
				   temp->signal == SIGTERM) {
				env_destroy(curenv);
			}
			if (temp != curenv->signal_list.head) {
				pre_temp->next = temp->next;
			} else {
				curenv->signal_list.head = temp->next;
			}
			curenv->signal_list.len--;
			if (flag) {
				if (curenv->cur_signal == NULL) {
					curenv->cur_signal = temp;
					curenv->cur_signal->next = NULL;
				} else {
					temp->next = curenv->cur_signal;
					curenv->cur_signal = temp;
				}
			}
			// env_pop_tf(&(curenv->env_tf), curenv->env_asid);
		}
	}
}
