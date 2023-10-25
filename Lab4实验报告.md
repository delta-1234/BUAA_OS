# Lab4实验报告

## 思考题

### Thinking 4.1

> **思考并回答下面的问题： **
>
> **• 内核在保存现场的时候是如何避免破坏通用寄存器的？ **
>
> **• 系统陷入内核调用后可以直接从当时的 $a0-$a3 参数寄存器中得到用户调用 msyscall 留下的信息吗？ **
>
> **• 我们是怎么做到让 sys 开头的函数“认为”我们提供了和用户调用 msyscall 时同样的参数的？ **
>
> **• 内核处理系统调用的过程对 Trapframe 做了哪些更改？这种修改对应的用户态的变化是什么？**

- 通过查看`stackframe.h`中的SAVE_ALL宏实现，内核保存现场先是将栈指针sp保存在k0寄存器中，之后利用sp将通用寄存器的值保存在内存的栈中，最后利用k0还原sp，因为k0寄存器在MIPS的约定中仅在内核态下使用，所以k0原本的值可以抛弃，这样就完成了用户态转内核态时的现场保护
- 可以，因为a0-a3寄存器中存储的就是函数传递的参数
- 利用a0-a3寄存器和保存在内存中栈的参数，将用户调研的msyscall的参数传递给了sys打头的一系列内核函数
- 将cp0_epc的值加4，使返回用户态时继续执行syscall下一条指令，并将sys函数返回值存入v0寄存器，使用户态获得syscall的返回值

---



### Thinking 4.2

> **思考 envid2env 函数: 为什么 envid2env 中需要判断 e->env_id != envid 的情况？如果没有这步判断会发生什么情况？**

因为使用env_alloc申请的的env控制块中的env_id是用mkenvid函数获得的，可能出现envid还未被alloc的情况，所以这种情况下e->env_id != envid，如果不加以判断，会出现调用未被alloc的env控制块，出现不可预料的错误

---



### Thinking 4.3

>**思考下面的问题，并对这个问题谈谈你的理解：请回顾 kern/env.c 文件 中 mkenvid() 函数的实现，该函数不会返回 0，请结合系统调用和 IPC 部分的实现与 envid2env() 函数的行为进行解释。 **

在系统调用和IPC的实现过程中需要在用户态获取到当前进程，但我们没办法直接读到内核态的curenv，所以可以使用envid为0作为参数获取到当前进程，因为正常进程的envid不会为0，所以在envid2env函数中可以特判传入参数为0的情况，返回当前进程

----



### Thinking 4.4

> **关于 fork 函数的两个返回值，下面说法正确的是： **
>
> **A、fork 在父进程中被调用两次，产生两个返回值 **
>
> **B、fork 在两个进程中分别被调用一次，产生两个不同的返回值 **
>
> **C、fork 只在父进程中被调用了一次，在两个进程中各产生一个返回值 **
>
> **D、fork 只在子进程中被调用了一次，在两个进程中各产生一个返回值 **

应该选C，fork函数只有父进程调用，只是在两个进程中的返回值不同

----



### Thinking 4.5

> **我们并不应该对所有的用户空间页都使用 duppage 进行映射。那么究竟哪些用户空间页应该映射，哪些不应该呢？请结合 kern/env.c 中 env_init 函数进行的页面映射、include/mmu.h 里的内存布局图以及本章的后续描述进行思考。**

UTOP及以上的空间在`env_init`中就已经完成了映射。USTACKTOP∼UTOP之间为Invalid memory和user exception stack，无需映射。因此，只需要映射USTACKTOP以下的内存空间。

----



### Thinking 4.6

> **在遍历地址空间存取页表项时你需要使用到 vpd 和 vpt 这两个指针，请参考 user/include/lib.h 中的相关定义，思考并回答这几个问题： **
>
> **• vpt 和 vpd 的作用是什么？怎样使用它们？ **
>
> **• 从实现的角度谈一下为什么进程能够通过这种方式来存取自身的页表？ **
>
> **• 它们是如何体现自映射设计的？ **
>
> **• 进程能够通过这种方式来修改自己的页表项吗？**

- vpt指向用户页表，vpd指向用户页目录，它们作用是在用户态下访问虚拟地址对应的页表项或者页目录项。使用`vpt[VPN(va)]`就可以访问va对应的页表项，使用`vpd[va >> 10]`就可以访问va对应的页目录项
- 因为进程使用的都是虚拟地址，页表的虚拟地址是连续的，所以可以用类似数组的方式来存取自身页表
- vpd的值为`(UVPT+(UVPT>>12)*4)`，意味以`UVPT`为基地址，向偏移`(UVPT>>12)`项，得到的也就是`UVPT`对应的页表项。将其作为vpd看待，即认为这一页映射了整个页表，可作为页目录。说明页表中存在某一页映射了整个页表，体现了自映射
- 不能，进程只能读页表，页表的管理由操作系统进行

---



### Thinking 4.7

> **在 do_tlb_mod 函数中，你可能注意到了一个向异常处理栈复制 Trapframe 运行现场的过程，请思考并回答这几个问题： **
>
> **• 这里实现了一个支持类似于“异常重入”的机制，而在什么时候会出现这种“异常重 入”？ **
>
> **• 内核为什么需要将异常的现场 Trapframe 复制到用户空间？**

- 在处理页写入异常的时候可能发生页写入异常，所以我们需要考虑异常重入
- 因为页写入异常的实际处理部分是在用户态进行的，所以需要将异常的现场复制到用户空间

----



### Thinking 4.8

> **在用户态处理页写入异常，相比于在内核态处理有什么优势？ **

在用户态处理页写入异常使得对其的处理对程序员不透明，程序员可以通过软件的方式自行定义解决页写入异常的方式，这使得页写入异常在处理上更灵活

----



### Thingking 4.9

> **请思考并回答以下几个问题： **
>
> **• 为什么需要将 syscall_set_tlb_mod_entry 的调用放置在 syscall_exofork 之前？**
>
> **• 如果放置在写时复制保护机制完成之后会有怎样的效果？**

- 在syscall_exofork函数执行的过程中需要env_alloc，此过程中可能发生页写入异常，所以需要先设置页写入异常处理函数入口
- 因为在syscall_exofork函数执行的过程可能发生页写入异常，如果此时未设置页写入异常处理函数，内核无法处理可能发生崩溃

----



## 实验难点

### Exercise 4.1

使用syscall指令即可，注意在函数最后使用`jr ra`进行返回

---



### Exercise 4.2

do_syscall函数作用是传递syscall的参数，执行对应的系统调用函数。根据a0寄存器中值决定需要使用的内核系统调用函数，从通用寄存器a1-a3和栈中读出需要的五个参数，并将v0寄存器值设置为系统调用函数的入口地址

---



### Exercise 4.3

首先应该判断envid是否为0，如果为0应该返回当前进程而不进行其他任何操作，之后需要判断checkperm的值，如果有效则需要保证返回的进程一定为当前进程的或者当前进程的子进程，否则返回异常值，如果无效则不需要检查

----



### Exercise 4.4 & 4.5 & 4.6

这三个函数实现比较简单，利用envid2env函数和我们在lab2中编写的page系列函数即可，注意page系列函数的异常返回值即可

---



### Exercise 4.7

调用schedule函数进行进程调度，注意设置yield参数为1，表示当前立即放弃当前进程的执行

---



### Exercise 4.8

注意sys_ipc_recv和sys_ipc_try_send两个函数中所有虚拟地址需要特判为0的情况，在sys_ipc_try_send中如果srcva为0，则表示我们只传递value的值给接收进程，而不需要传递物理页面，也就是说我们需要做以下判断：

```c
if (srcva != 0) {
    /* Exercise 4.8: Your code here. (8/8) */
    Pte *ppte = NULL;
    struct Page * pa = page_lookup(curenv->env_pgdir, srcva, &ppte);
    if (pa == NULL) {
        return -E_INVAL;
    }
    try(page_insert(e->env_pgdir, e->env_asid, pa, e->env_ipc_dstva, perm));
}
```

----



### Exercise 4.9

sys_exofork函数的目的是新建一个子进程，是我们fork函数的核心组成部分，新建的子进程应该具有和父进程相同的上下文，所以要将父进程的栈复制给子进程，同时设置子进程为ENV_NOT_RUNNABLE，等待后续步骤进行唤醒。

----



### Exercise 4.10

duppage作用是将父进程拥有的物理页共享给子进程，并将可写的页设置为写时复制。

```C
static void duppage(u_int envid, u_int vpn) {
	int r;
	u_int addr;
	u_int perm;

	addr = vpn * BY2PG;
	perm = (vpt[vpn]) & 0xfff;

	if ((perm & PTE_D) == 0 || (perm & PTE_LIBRARY) || (perm & PTE_COW)) {
		syscall_mem_map(0, (void *)addr, envid, (void *)addr, perm);
        //当该页面不可写，只需要直接映射给子进程
	} else {
		perm = (perm | PTE_COW) & (~PTE_D);
		syscall_mem_map(0, (void *)addr, envid, (void *)addr, perm);
		syscall_mem_map(0, (void *)addr, 0, (void *)addr, perm);
        //页面可写，需要更新权限位，先映射给子进程，再映射给父进程
	}
}
```

----



### Exercise 4.11 & 4.12

do_tlb_mod函数和sys_set_tlb_mod_entry 函数都是用于处理页写入异常的函数，它们是内核函数，目的是传递参数和为用户态的页写入异常处理函数设置入口，补全代码部分比较简单。

---



### Exercise 4.13 & 4.14

cow_entry这个函数式用户态中真正用于处理页写入异常中写时复制页面的函数，编写时注意va不一定是页对齐的。sys_set_env_status 用于设置进程的状态，注意在状态发生转变时要将其插入或者移出调度队列。

---



### Exercise 4.15

```C
int fork(void) {
	u_int child;
	u_int i;
	extern volatile struct Env *env;
	
    //如果父进程未设置页写入异常处理函数，先设置
	if (env->env_user_tlb_mod_entry != (u_int)cow_entry) {
		try(syscall_set_tlb_mod_entry(0, cow_entry));
	}

	child = syscall_exofork(); //创建子进程
    //子进程执行
	if (child == 0) {
		env = envs + ENVX(syscall_getenvid());
		return 0;
	}
	
    //父进程将当前内存部分页面共享给子进程
	int va = 0;
	while (va < (USTACKTOP >> 12)) {
        //只有有效页需要映射，提高效率
		if ((vpd[va >> 10] & PTE_V) && (vpt[va] & PTE_V)) { 
			duppage(child, va);
		}
		va++;
	}

    //设置子进程页写入异常处理函数
	try(syscall_set_tlb_mod_entry(child, cow_entry));
    //设置子进程页状态为可运行，加入调度队列
	try(syscall_set_env_status(child, ENV_RUNNABLE));
	return child;
}
```

----



## 心得体会

lab4实验跨越两周，在复杂度上相比起之前的实验有所增加，同时在调试上的难度也增加了许多，我采用的调试方法依然是在内核态中使用printk函数打印信息，在用户态中使用debugf打印信息，综合输出判断bug发生位置，fork函数是目前lab中涉及到的最复杂的函数，为了解决其出现的bug，我不得不在所有相关函数中增加输出信息，这样虽然效率比较低，但个人认为相比起使用GXemul调试更加直观。填写完实验代码不算困难，在群里同学和助教的帮助下也能比较快速的发现问题，但更重要的是理解用户态和内核态的不同，之前的lab中我们接触到的基本都是内核态函数的编写，这次的lab涉及到了两种状态之间的转换，系统调用函数是在用户态中使用的，而我们需要将其层层传递至内核态，由内核函数真正执行。
