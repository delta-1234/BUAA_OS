#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_ov(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
	[12] = handle_ov,
    [2 ... 3] = handle_tlb,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
    
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */

void do_ov(struct Trapframe *tf) {
	u_long epc = tf->cp0_epc;
	struct Env *e = curenv;
	Pte *pte = NULL;
	struct Page * page = page_lookup(e->env_pgdir, epc, &pte);
	u_int zl = *((u_int *)((KADDR(PTE_ADDR(*pte))) | (epc & 0xfff)));
	u_int addi = (15) << 28;
	u_int d, s, t, imm;
	e->env_ov_cnt++;
	s = (zl << 6) >> 27;
	t = (zl << 11) >> 27;
	// printk("%x", zl);
	// panic("%d", zl & addi);
	if ((zl & addi) != 0) {
		imm = (zl << 16) >> 16;
		tf->regs[t] = tf->regs[s] / 2 + imm / 2;
		tf->cp0_epc = epc + 4;
		printk("addi ov handled\n");
	} else {
		if ((zl & 15) == 0) {
			*((u_int *)((KADDR(PTE_ADDR(*pte))) | (epc & 0xfff))) = zl | 1;
			printk("add ov handled\n");
		} else if ((zl & 15) == 2) {
			*((u_int *)((KADDR(PTE_ADDR(*pte))) | (epc & 0xfff))) = zl | 1;
			printk("sub ov handled\n");
		}
	}
}

void do_reserved(struct Trapframe *tf) {
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}
