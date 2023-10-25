# Lab2实验报告

## 思考题

### Thinking 2.1

在编写的C程序中，指针中存储的是虚拟地址；MIPS汇编程序中lw和sw使用的地址直接由CPU处理，所以也是虚拟地址



----

### Thinking 2.2

- 从可重用性的角度来说，使用宏来实现链表可以达到类似泛型的效果，使用者可以利用同一个宏创建不同类型的链表而不需要重复链表结构体的定义，大大简化了编写代码的重复度，也提供高了代码的可重用性，提高了代码可读性

- 实验环境中的 /usr/include/sys/queue.h文件中包含了多种数据结构，其中单向链表（Singly-linked List）、单向循环链表（Simple queue）和双向循环链表（Circular queue）三者对外提供的接口方法基本一致，但实现上存在一些差异。单向循环链表在单向链表的基础上将尾节点指向了头结点；双向循环链表中不光存储着指向下个节点的指针，还存储着指向上个节点的指针。

  - 单向链表插入复杂度为O(1)，删除复杂度为O(n)
  - 单向循环链表插入复杂度为O(1)，删除复杂度为O(n)
  - 双向循环链表插入复杂度为O(1)，删除复杂度为O(1)

  可以发现双向循环链表在插入和删除的性能都很高，对比我们实验中的双向链表，在性能上差别不大，只是Linux提供的双向循环链表中cqe_prev指向上一个节点，但我们的实验中，le_prev指向的是上一个节点的le_next指针，这样在删除操作时只要更改le_prev就可以，简化了删除操作，在理论上拥有比双向循环链表更快的删除速度



-----

### Thinking 2.3

queue.h和pmpa.h中关于Page_list的相关内容如下：

```C
//pmap.h
LIST_HEAD(Page_list, Page);
typedef LIST_ENTRY(Page) Page_LIST_entry_t;
struct Page {                                                          
	Page_LIST_entry_t pp_link; /* free list link */
    u_short pp_ref;
};

//queue.h
#define LIST_HEAD(name, type)                                                                            \
	struct name {                                                                                    \
		struct type *lh_first; /* first element */                                               \
	}

#define LIST_ENTRY(type)                                                                                 \
    struct {                                                                                         \
        struct type *le_next;  /* next element */                                                \
        struct type **le_prev; /* address of previous next element */                            \
	}
```

通过观察以上内容，我们可以比较简单的展开，最后结果为：

```C
struct Page_list{
    struct {
        struct {
            struct Page *le_next;
            struct Page **le_prev;
        } pp_link;
        u_short pp_ref;
    }* lh_first;
}
```

所以选择C



----

### Thinking 2.4

- 因为一台计算机可以同时运行多个进程，不同的进程使用可能相同的虚拟地址，但这些虚拟地址映射到的物理地址却可能不同，所以需要ASID进行标识，只有当前运行的进程和ASID匹配时，才能正确映射到需要的物理地址
- ASID共有6位，故能容纳不同地址空间的最大数量为64



----

### Thinking 2.5

- tlb_invalidate通过`tlb_out(PTE_ADDR(va) | (asid << 6))`调用了tlb_out
- tlb_invalidate在页表更新时删除指定虚拟地址在TLB中的旧表项
- 逐行解释如下：

```assembly
LEAF(tlb_out)
.set noreorder #汇编器设置为noreorder模式，不对指令进行重新排序，原样执行
	mfc0    t0, CP0_ENTRYHI #将EntryHi寄存器中值写入通用寄存器t0，保存原值
	mtc0    a0, CP0_ENTRYHI #将a0寄存器中值，即传入的参数写入EntryHi
	nop #解决数据冒险
	tlbp #根据 EntryHi 中的 Key（包含 VPN 与 ASID），查找 TLB 中与之对应的表项，并将表项的索引存入 Index 寄存器（若未找到匹配项，则 Index 最高位被置 1）
	nop #解决数据冲突
	mfc0    t1, CP0_INDEX #将Index寄存器中值，即查找的结果保存在通用寄存器t1中
.set reorder #恢复reorder模式，汇编器自动调整指令顺序进行优化
	bltz    t1, NO_SUCH_ENTRY #未找到t1为负数，跳转到NO_SUCH_ENTRY
.set noreorder #设置为noreorder模式
	mtc0    zero, CP0_ENTRYHI #找到va，将TLB对应项清零
	mtc0    zero, CP0_ENTRYLO0 #清零
	nop #消除数据冲突
	tlbwi #以 Index 寄存器中的值为索引，将此时 EntryHi 与 EntryLo 的值写到索引指定的 TLB 表项中
.set reorder #恢复reorder模式

NO_SUCH_ENTRY:
	mtc0    t0, CP0_ENTRYHI #未找到，恢复原值
	j       ra #返回
END(tlb_out)
```



----

### Thinking 2.6

> 简单了解并叙述 X86 体系结构中的内存管理机制，比较 X86 和 MIPS 在内存管理上的区别

x86中有两种内存管理方式，分页和分段，而MIPS中只有分页一种，并且在x86中地址也分为逻辑地址、线性地址和物理地址三种，分段可以将逻辑地址转化为线性地址，分页将线性地址转化为物理地址，分段是强制的，而分页是可选的。x86有实模式和保护模式两种，实模式只在机器刚刚启动时使用，之后便切换到保护模式，而MIPS中没有这两种模式区分，而是存在内核态和用户态两种状态。x86在实模式下获取物理地址的方式很简单，将CS段寄存器的值左移4位然后与IP寄存器中的值相加，得出的结果（20位的地址）就是要访问的内存的物理地址；x86保护模式下物理地址计算比较复杂，简单的可以分为以下四步：

1. 从段寄存器中取出选择子
2. 通过段选择子的索引信息以及GDTR记录的GDT的地址找到段描述符的地址
3. 检查权限等信息，完成后取得段的物理基址
4. 将物理基址与EIP寄存器中的偏移量相加得到实际物理地址



----

### Thinking A.1

- $三级页表目录基地址 = PT_{base} | PT_{base} >> 9 $
- $ 自映射页目录项 = PT_{base}| PT_{base} >> 9 | PT_{base} >> 18 | PT_{base} >> 27$



-----

## 难点分析

### Exercise 2.1

> **请参考代码注释，补全 mips_detect_memory 函数。在实验中，从外设中获取了硬件可用内存大小 memsize，请你用内存大小 memsize 完成总物理页数 npage 的初始化。 **



我们可以发现有一个宏`BY2PG`存储的就是页的大小也就是4096，所以只需要用总大小`memsize`除以每个页大小就可以得到总页面数，注意不能使用`sizeof(Page)`来表示页面大小，Page结构体只是页面控制器，并不是真实的页面存储结构体



----

### Exercise 2.2

> **完成 include/queue.h 中空缺的函数 LIST_INSERT_AFTER。其功能是将一个元素插入到已有元素之后，可以仿照 LIST_INSERT_BEFORE 函数来实现。 **



根据提示，我们可以参照LIST_INSERT_BEFORE来补全，在补全前先要了解几个相关宏的用法：

```C
#define LIST_INSERT_AFTER(listelm, elm, field)                                                     \
	do {															\
		LIST_NEXT((elm), field) = LIST_NEXT((listelm), field);		\ 
		/*将elm的le_next指针指向listelm后一个节点*/ 					\
		if (LIST_NEXT((listelm), field) != NULL) {					\ 
            /*如果listelm下一个节点非空，则将下一个节点的le_prev指向elm的le_next*/  		\
			LIST_NEXT((listelm), field)->field.le_prev = &LIST_NEXT((elm), field);	\
		}															\
		LIST_NEXT((listelm), field) = (elm);							\ 
        /*将listelm的le_next指向elm*/									\
		(elm)->field.le_prev = &LIST_NEXT((listelm), field);			\ 
        /*将elm的le_prev指针指向listelm的le_next*/							\
	} while (0)
```



-----

### Exercise 2.3

> **完成 page_init 函数。请按照函数中的注释提示，完成上述三个功能。此外，这里也给出一些提示：**
>
> 1. **使用链表初始化宏 LIST_INIT。**
>
> 2. **将 freemem 按照 BY2PG 进行对齐（使用 ROUND 宏为 freemem 赋值）。**
>
> 3. **将 freemem 以下页面对应的页控制块中的 pp_ref 标为 1。**
>
> 4. **将其它页面对应的页控制块中的 pp_ref 标为 0 并使用 LIST_INSERT_HEAD 将其插入空闲链表。**



首先应该使用`LIST_INIT(&page_free_list)`初始化一个链表，`page_free_list.lh_first`为指向链表头结点的指针；第二步使freemem地址页对齐，使用`freemem = ROUND(freemem, BY2PG)`就可以做到；第三步需要将freemem这个虚拟地址以下的页面初始化，并将其引用赋1，表示其已经被使用；第四步将freemem这个虚拟地址以上的页面初始化，并将其引用赋0，表示其为空闲页面，利用`LIST_INSERT_HEAD`将其加入到空闲页面链表（page_free_list）中

```C
LIST_INIT(&page_free_list);
freemem = ROUND(freemem, BY2PG);
int i;
for (i=0;page2kva(&pages[i]) < freemem;i++) { //利用page2kva函数可以将页面转换为其虚拟地址，与freemem进行比较
	pages[i].pp_ref = 1;
}
for (i=i;i < npage;i++) { //总页面数为npage
	pages[i].pp_ref = 0;
	LIST_INSERT_HEAD(&page_free_list, &pages[i], pp_link);
}
```



----

### Exercise 2.4

> **完成 page_alloc 函数。在 page_init 函数运行完毕后，在 MOS 中如果想申请存储空间，都是通过这个函数来申请分配。该函数的逻辑简单来说，可以表述为：**
>
> 1. **如果空闲链表没有可用页了，返回异常返回值。**
> 2. **如果空闲链表有可用的页，取出第一页；初始化后，将该页对应的页控制块的**
>
> **地址放到调用者指定的地方。填空时，你可能需要使用链表宏 LIST_EMPTY 或函数 page2kva。**



首先应该使用`LIST_EMPTY(&page_free_list)`判断是否有空闲页面，若没有则返回异常值，我们可以从指导书中得到此异常值为`-E_NO_MEM`；如果有空闲页面，则使用`LIST_FIRST(&page_free_list)`取出一个空闲页并赋值给pp，之后使用`LIST_REMOVE(pp, pp_link)`把pp从链表中去除；最后使用`memset((void *)page2kva(pp), 0, BY2PG)`初始化pp对应的页，最后将new指向pp即可



----

### Exercise 2.5

> **完成 page_free 函数。**
>
> **提示：使用链表宏 LIST_INSERT_HEAD，将页结构体插入空闲页结构体链表。**

 

使用`LIST_INSERT_HEAD(&page_free_list, pp, pp_link)`将空闲页面插入即可



----

### Exercise 2.6

> **完成 pgdir_walk 函数。**
>
> **该函数的作用是：给定一个虚拟地址，在给定的页目录中查找这个虚拟地址对应的页表项，将其地址写入 *ppte。**



此函数在之后的编写过程中十分关键，如果此函数编写错误会出现很多奇怪的bug。题目表述上有些不清晰，其实此函数作用就是在根据给定的虚拟地址查找对应的二级页表表项的地址，如果未找到且创建指令为真，则创建一个二级页表；如果未找到且创建指令为假，则返回空指针

```C
static int pgdir_walk(Pde *pgdir, u_long va, int create, Pte **ppte) {
	Pde *pgdir_entryp;
	struct Page *pp;

	pgdir_entryp = pgdir + PDX(va); //一级页表基址（pgdir）加上页目录偏移量就为虚拟地址对应的一级页表表项的地址

	if ((*pgdir_entryp & PTE_V) == 0) { //对应页表项不存在或者无效
		if (create != 0) { //创建指令为真，则创建
			if (page_alloc(&pp) == 0) { //有空闲页用于创建
				pp->pp_ref += 1; //该页面被使用，引用加一
				*pgdir_entryp = page2pa(pp) | PTE_D | PTE_V; //设置页面
			} else { //无空闲页，则返回错误
				return -E_NO_MEM;
			}
		} else {
			*ppte = NULL; //创建指令为假，ppte指向空
			return 0;
		}
	}
	
	*ppte = ((Pte *)(KADDR(PTE_ADDR(*pgdir_entryp)))) + PTX(va); //利用PTE_ADDR从pgdir_entryp对应页表项中读出二级页表物理地址基址，再使用KADDR将其转化为虚拟地址，加上二级页表偏移量，得到虚拟地址对应的二级页表表项
	return 0;
}
```



----

### Exercise 2.7

> **完成 page_insert 函数（补全 TODO 部分）。 **



在页面插入后，需要先使用`tlb_invalidate(asid, va)`删除TLB中原有的va对应的页面，之后查找对应的二级页表表项，找不到则创建，此过程使用pgdir_walk函数即可完成，最后将要插入的页面赋值即可



----

### Exercise 2.8

> **完成 kern/tlb_asm.S 中的 tlb_out 函数。该函数根据传入的参数（TLB 的Key）找到对应的 TLB 表项，并将其清空。具体来说，需要在两个位置插入两条指令，其中一个位置为 tlbp，另一个位置为 tlbwi。因流水线设计架构原因，tlbp 指令的前后都应各插入一个 nop 以解决数据冒险。**



在相应位置填上tlbp和tlbwi即可，具体解释在思考题中给出了



----

### Exercise 2.9

> **完成 kern/tlbex.c 中的 _do_tlb_refill 函数。**



按照所给的提示补全

```C
while (page_lookup(cur_pgdir, va, &pte) == NULL) {
	passive_alloc(va, cur_pgdir, asid);
}
```



----

### Exercise 2.10

> **完成 kern/tlb_asm.S 中的 do_tlb_refill 函数。 **



在对应位置填上tlbwr即可



----

## 实验体会

本次课下实验难度较大，主要原因是对页式内存管理的了解不够充分，同时实验代码中存在着许多的宏定义，而这些宏定义有一部分在实验指导书中没有解释用法，只能自己去看源码猜测其用法。内存管理中牵扯到两级页表和虚拟地址和物理地址的转换，光看指导书很难搞明白什么地方是虚拟地址什么地方是物理地址，而实验中又有多个函数用于转换，在阅读代码上花费了大量的时间。同时此次实验中还出现了许多的bug，虽然通过报错能获得一些信息，但在具体锁定bug位置上有很大的难度，但本人对GXemul调试不太熟练，在debug阶段也花费了大量的时间
