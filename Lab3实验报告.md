# Lab3实验报告

## 思考题

### Thinking 3.1

> **请结合 MOS 中的页目录自映射应用解释代码中 e->env_pgdir[PDX(UVPT)] = PADDR(e->env_pgdir) | PTE_V 的含义**

UVPT在MOS中代表的是用于存储页表的4MB内存的起始虚拟地址，因为我们采用的两级页表映射机制，4MB内存中一共有1024个4KB的页表，其中一个页表即页目录，其映射到的4MB空间刚好是这1024个页表所占空间。这一行代码意义为：$PDX(UVPT)$为虚拟地址UVPT对应的页目录索引，所以$e->env\_pgdir[PDX(UVPT)]$就为虚拟地址UVPT对应的页目录表项，因为页目录自映射，此表项映射到的二级页表应该为页目录自身，所以将其赋值为$PADDR(e->env\_pgdir)$并将权限位设置为有效。

-----



### Thinking 3.2

> **elf_load_seg 以函数指针的形式，接受外部自定义的回调函数 map_page。 请你找到与之相关的 data 这一参数在此处的来源，并思考它的作用。没有这个参数可不可 以？为什么？**

data的来源为load_icode函数传进来的struct Env *e，为需要加载elf文件的进程，此参数的作用为提供給外部定义的回调函数map_page作为参数使用，没用这个参数是不可以的，因为在elf_load_seg中需要使用外部定义的回调函数map_page，而map_page又需要知道当前的需要加载的进程，没用传进来的data参数，map_page无法运行。

----



### Thinking 3.3

> **结合 elf_load_seg 的参数和实现，考虑该函数需要处理哪些页面加载的情况。**

结合源码，该函数需要处理页面加载的情况有：

1. 加载进去的elf文件是否是可写的  

2. 需要被加载到的虚拟地址va是不是页对齐的
3. 需要拷贝的数据长度 bin_size 是不是页对齐的

-----



### Thinking 3.4

> **思考上面这一段话，并根据自己在 Lab2 中的理解，回答： **
>
> **•  你认为这里的 env_tf.cp0_epc 存储的是物理地址还是虚拟地址?**

结合我在计组中的学习，CP0寄存器中的EPC存储的异常处理完需要返回的PC值，所以将其赋值为e_entry让CPU在异常处理结束后跳转到我们要运行的程序入口进行执行，因为CPU接收的都是虚拟地址，所以这里的env_tf.cp0_epc 存储的一定是虚拟地址。

----



### Thinking 3.5

> **试找出 0、1、2、3 号异常处理函数的具体实现位置。8 号异常（系统调用） 涉及的 do_syscall() 函数将在 Lab4 中实现。 **

- 0号异常处理函数 handle_int，具体实现在汇编文件kern/genex.S中，函数的入口位置为NESTED(handle_int, TF_SIZE, zero)

- 1号异常处理函数 handle_mod，具体实现在汇编文件kern/genex.S中， 具体函数还未实现，函数为do_tlb_mod
- 2号和3号异常函数都为 handle_tlb，具体实现在汇编文件kern/genex.S中，具体函数还未实现，函数为do_tlb_refill

---



### Thinking 3.6

> **阅读 init.c、kclock.S、env_asm.S 和 genex.S 这几个文件，并尝试说出 enable_irq 和 timer_irq 中每行汇编代码的作用。**

```assembly
LEAF(enable_irq)
	li      t0, (STATUS_CU0 | STATUS_IM4 | STATUS_IEc)
	mtc0    t0, CP0_STATUS
	# 设置SR寄存器，将CU0置1使在用户态下也可以使用一些特权指令，IM4置1表示可以相应4号中断，IEc置1表示当前开启中断
	jr      ra
END(enable_irq)
```

```assembly
timer_irq:
	sw      zero, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK)
	# 写入0，应该是表示暂时关闭时钟中断，在进行异常处理时不能产生时钟中断
	li      a0, 0
	# 传递给scedule函数的参数即yield的值
	j       schedule
	# 跳转到schedule函数
```

----



### Thinking 3.7

> **阅读相关代码，思考操作系统是怎么根据时钟中断切换进程的。**

CPU在运行时如果接受到时钟中断，就会立即停止指令的执行，跳转到异常处理程序，并陷入内核态，并将产生中断的原因保存在CP0中的Cause寄存器中，操作系统通过读取Cause寄存器可以得知中断的原因，如果是时钟中断，就通过timer_irq函数调用schedule函数进行进程的切换，schedule函数结束后恢复CPU执行，跳出异常处理程序。

----



## 难点分析

### Exercise 3.1

> **完成 env_init 函数。 实现 Env 控制块的空闲队列和调度队列的初始化功能。请注意，你需要按倒序将所有控制块插入到空闲链表的头部，使得编号更小的进程控制块被优先分配。**

```C
	LIST_INIT(&env_free_list); //初始化env_free_list为单向链表
	TAILQ_INIT(&env_sched_list); //初始化env_sched_list为双向链表
	for (i = NENV - 1;i >= 0;i--) { //倒序插入链表头
		envs[i].env_status = ENV_FREE; //初始所有进程程控制块状态都为空闲
		LIST_INSERT_HEAD(&env_free_list, &envs[i], env_link);
	}
```

----



### Exercise 3.2

> **请你结合 env_init 中的使用方式，完成 map_segment 函数。**

在循环中使用page_insert函数即可，注意需要将物理地址使用pa2page函数转化为Page *

---



### Exercise 3.3

> **完成 env_setup_vm 函数。 **
>
> **仔细阅读前文的提示理解一个进程虚拟地址空间的分布，根据注释完成函数，实现初始化一个新进程地址空间的功能。**

首先pp_ref加一，之后将此进程的env_pgdir设置为p对应的地址即可，具体可以使用

`e->env_pgdir = (Pde *)page2kva(p)`完成

---



### Exercise 3.4

> **完成 env_alloc 函数。 env_alloc 函数实现了申请并初始化一个进程控制块的功能。这里给出如下提示：** 
>
> 1. **回忆 Lab2 中的链表宏 LIST_FIRST、LIST_REMOVE，实现在 env_free_list 中申请空闲进程控制块。 **
> 2.  **用 env_setup_vm 初始化新进程的地址空间。**
> 3.  **仔细阅读前文中对与 Lab3 相关的域的介绍，思考相关域的恰当赋值。**

首先需要判断env_free_list是否为空，为空的话需要返回`-E_NO_FREE_ENV`，之后使用`try(env_setup_vm(e))`建立进程对应的虚拟空间，注意要使用try进行错误捕捉；之后按照提示使用mkenvid和asid_alloc生成env_id和env_asid即可，注意asid_alloc不一定成功；最后将e从env_free_list中移除即可。

----



### Exercise 3.5

> **完成 kern/env.c 中的 load_icode_mapper 函数。**
>
> **提示：可能使用到的函数有 page_alloc，page_insert，memcpy。 **

具体补充部分如下：
```c
r = page_alloc(&p);
if (r == -E_NO_MEM) { //注意异常情况出现
    return -E_NO_MEM;
}
if (src != NULL) {
    memcpy((void *)(page2kva(p) + offset), src, len); 
    //注意并不是p直接加offset，而是其对应的虚拟地址
}
```

----



### Exercise 3.6

> **根据注释的提示，完成 kern/env.c 中的 load_icode 函数。**

将epc设置为进程入口即可，`e->env_tf.cp0_epc = ehdr->e_entry`

---



### Exercise 3.7

> **完成 env_create 函数。 **
>
> **根据提示，理解并恰当使用前面实现的函数，完成 kern/env.c 中 env_create 函数的填写，实现创建一个新进程的功能。**

```C
struct Env *env_create(const void *binary, size_t size, int priority) {
	struct Env *e;
	try(env_alloc(&e, 0)); //注意所有alloc类型的错误返回值
	e->env_status = ENV_RUNNABLE; //创建出的进程应该设置为可执行
	e->env_pri = priority; //设置优先级
	load_icode(e, binary, size); //将可执行文件加载到进程中
	TAILQ_INSERT_HEAD(&env_sched_list, e, env_sched_link); //将该进程加入进程管理队列中进行管理
	return e;
}
```

----



### Exercise 3.8

> **完成 env_run。 **
>
> **仔细阅读前文讲解，并根据注释填写 kern/env.c 中的 env_run 函数。** 

将全局变量cur_pgdir设置为当前进程的页目录地址，之后按照提示调用env_pop_tf函数来保存上下文。

---



### Exercise 3.9

> **补充 kern/entry.S。 **
>
> **理解异常分发代码，并将异常分发代码填至 kern/entry.S 恰当的部分。**

填写的代码表示将CP0中Cause寄存器值保存在t0寄存器中，将异常码部分复位，将t0作为参数交给异常处理程序进行处理。

----



### Exercise 3.10

> **补全 kernel.lds。 **
>
> **根据前文讲解将 kernel.lds 代码补全使得异常发生后可以跳到异常分发代码。**

通过lds脚本将异常处理入口放置在正确的地址上，一般异常的入口为0x8000_0000，而TLB Miss异常处理入口在0x8000_0080

----



### Exercise 3.11

> **补充 kclock_init 函数。 **
>
> **通过上面的描述，补充 kern/kclock.S 中的 kclock_init 函数。**

使用`sw     t0, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_HZ)`将时钟频率设置为200，其中KSEG1 | DEV_RTC_ADDRESS 是模拟器（GXemul）映射实时 钟的位置，而DEV_RTC_HZ 表示设置实时钟中断的频率。

----



### Exercise 3.12

>**完成 schedule 函数。 **
>
>**根据注释，填写 kern/sched.c 中的 schedule 函数实现切换进程的功能，使得进程 能够被正确调度。 **

这部分理解上比较困难，并且逻辑的正确很重要，这里我使用的是助教给出的第一个思路，首先判断 e 是否仍为就绪状态（ENV_RUNNABLE），若是则将其移动到调度队列尾部；然后再选中调度队列首部的进程（但不移出队列），将其赋值给 e，作为当前需要调度的进程。具体代码如下：

```C
void schedule(int yield) {
	static int count = 0;
	struct Env *e = curenv;
	if (yield || count <= 0 || e == NULL || e->env_status != ENV_RUNNABLE) { //进程切换的条件
		if (e != NULL && e->env_status == ENV_RUNNABLE) { //判断前需要保证e不为NULL
			TAILQ_REMOVE(&env_sched_list, e, env_sched_link);
			TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);
		}
		e = TAILQ_FIRST(&env_sched_list);
		if (e == NULL) { //e为空表达没有可以运行的进程了
			panic("schedule: no runnable envs");
		}
		count = e->env_pri; //切换进程后将优先级设置为可占用的时间片数量
	}
	count--; //将count减1，表示运行过一个时间片了
	env_run(e); //运行进程
}

```

----



## 实验体会

本次课下实验补全代码的难度不大，提示给的十分详细，需要特别注意的是各种alloc类函数的错误返回值以及许多的异常情况。虽然代码补全的难度不算很大，但实际理解上还是比较困难，此次实验涉及到的函数很多，并且对要求对lab2的代码有很高的熟悉程度，在理解上比较困难，个人对多级页表以及物理地址和虚拟地址转换上还是有些一知半解。在debug方面，我基本放弃了使用GXemul进行调试，转而使用打印流，对我来说，与其使用我不熟悉的调试器，不如在可能出现bug的地方多printk直接将信息显示展现出来。
