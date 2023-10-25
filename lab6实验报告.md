# Lab6实验报告

## 思考题

### Thinking 6.1

> **示例代码中，父进程操作管道的写端，子进程操作管道的读端。如果现在想 让父进程作为“读者”，代码应当如何修改？**

应当修改为如下形式：

```C
#include <stdlib.h>
#include <unistd.h>

int fildes[2];
char buf[100];
int status;

int main(){
    status = pipe(fildes);

    if (status == -1 ) {
        printf("error\n");
    }

    switch (fork()) {
        case -1:
            break;

        case 0: /* 子进程 - 作为管道的写者 */
            close(fildes[0]); /* 关闭不用的读端 */
            write(fildes[1], "Hello world\n", 12); /* 向管道中写数据 */
            close(fildes[1]); /* 写入结束，关闭写端 */
            exit(EXIT_SUCCESS);

        default: /* 父进程 - 作为管道的读者 */
            close(fildes[1]); /* 关闭不用的写端 */
            read(fildes[0], buf, 100); /* 从管道中读数据 */
            printf("child-process read:%s",buf); /* 打印读到的数据 */
            close(fildes[0]); /* 读入结束，关闭读端 */
            exit(EXIT_SUCCESS);
    }
}
```

-----



### Thinking 6.2

> **上面这种不同步修改 pp_ref 而导致的进程竞争问题在 user/lib/fd.c 中 的 dup 函数中也存在。请结合代码模仿上述情景，分析一下我们的 dup 函数中为什么会出 现预想之外的情况？ **

dup函数的功能是将一个文件描述符（例如fd0）所对应的内容映射到另一个文件描述符（例如fd1）中。这个函数最终会将fd0和pipe的引用次数都增加1，将fd1的引用次数变为fd0的引用次数。若在复制了文件描述符页面后产生了时钟中断，pipe的引用次数没来的及增加，可能会导致另一进程调用`pipe_is_closed`，发现`pageref(fd[0]) = pageref(pipe)`，误以为读/写端已经关闭。

-----



### Thinking 6.3

> **阅读上述材料并思考：为什么系统调用一定是原子操作呢？如果你觉得不是所有的系统调用都是原子操作，请给出反例。希望能结合相关代码进行分析说明。 **

在系统调用的过程中禁止了时空中断，所以不会发生进程的切换，所以系统调用一定是原子操作，我们的实验中在init/start.S中规定了通过陷入进入内核态时禁止中断

```assembly
.text
EXPORT(_start)
.set at
.set reorder
	/* disable interrupts */
	mtc0    zero, CP0_STATUS

	/* set up the kernel stack */
	li sp, 0x80400000
	/* jump to mips_init */
	jal mips_init
```

----



### Thinking 6.4

> **仔细阅读上面这段话，并思考下列问题 **
>
> **• 按照上述说法控制 pipe_close 中 fd 和 pipe unmap 的顺序，是否可以解决上述场景的进程竞争问题？给出你的分析过程。 **
>
> **• 我们只分析了 close 时的情形，在 fd.c 中有一个 dup 函数，用于复制文件描述符。 试想，如果要复制的文件描述符指向一个管道，那么是否会出现与 close 类似的问 题？请模仿上述材料写写你的理解。**

- 可以解决，调换顺序之后，先增加pipe的引用，不会导致`pageref(fd[0]) = pageref(pipe)`，也就不会引起管道错误关闭
- dup也会出现类似的问题，具体分析在Thingking 6.2中进行了介绍，解决方法同close类似，调换fd和pipe的顺序即可

-----



### Thinking 6.5

> **思考以下三个问题。 **
>
> **• 认真回看 Lab5 文件系统相关代码，弄清打开文件的过程。 **
>
> **• 回顾 Lab1 与 Lab3，思考如何读取并加载 ELF 文件。 **
>
> **• 在 Lab1 中我们介绍了 data text bss 段及它们的含义，data 段存放初始化过的全 局变量，bss 段存放未初始化的全局变量。关于 memsize 和 filesize ，我们在 Note 1.3.4中也解释了它们的含义与特点。关于 Note 1.3.4，注意其中关于“bss 段并不在文 件中占数据”表述的含义。回顾 Lab3 并思考：elf_load_seg() 和 load_icode_mapper() 函数是如何确保加载 ELF 文件时，bss 段数据被正确加载进虚拟内存空间。bss 段 在 ELF 中并不占空间，但 ELF 加载进内存后，bss 段的数据占据了空间，并且初始值都是 0。请回顾 elf_load_seg() 和 load_icode_mapper() 的实现，思考这一点 是如何实现的？ **

- 用户调用**open**函数，实际上会转到**fsipc_open**函数中进行处理，**fsipc_open**会调用**fsipc**利用我们在lab4中实现的**ipc**向文件系统服务进程发送消息；文件系统服务进程接收到消息后，调用**serve**函数，分析请求的操作为open，之后调用**serve_open**函数，进一步调用文件系统服务进程中的**file_open**函数，file_open函数会调用**walk_path**函数进行路径的解析，最终使用**dir_loopup**函数找到文件的位置，返回打开的文件指针
- 读取elf文件在我们的实验中由main.c中调用**readelf**函数完成，实现了将一个二进制文件读取到内存中并保存指向文件头的指针。加载elf文件工作由**load_icode**函数完成，通过强制类型转换，可以将读取的二进制数组转换为elf文件头结构体，再将elf中各个段加载进来，完成加载
- 通过查看**elf_load_seg**函数内容，可以发现实际上一个elf文件大小只有**bin_size**，但通过一个变量p_memsz记录了一个elf文件加载后应该对应的大小**sgsize**，二者的差就是bss段，所以在加载前bss段不真正占有空间，直到在elf_load_seg中对`bin_size~sgsize`段，即bss段，直接进行了赋值，全部设置为0，这时bss段的数据才真正占据了空间。

---



### Thinking 6.6

> **通过阅读代码空白段的注释我们知道，将标准输入或输出定向到文件，需要 我们将其 dup 到 0 或 1 号文件描述符（fd）。那么问题来了：在哪步，0 和 1 被“安排”为 标准输入和标准输出？请分析代码执行流程，给出答案。**

在user/init.c中，在初始化整个操作系统用户空间的时候就将0和1设置为标准输入和标准输出

```C
// stdin should be 0, because no file descriptors are open yet
if ((r = opencons()) != 0) {
    user_panic("opencons: %d", r);
}
// stdout
if ((r = dup(0, 1)) < 0) {
    user_panic("dup: %d", r);
}
```

----



### Thinking 6.7

> **在 shell 中执行的命令分为内置命令和外部命令。在执行内置命令时 shell 不 需要 fork 一个子 shell，如 Linux 系统中的 cd 命令。在执行外部命令时 shell 需要 fork 一个子 shell，然后子 shell 去执行这条命令。 **
>
> **据此判断，在 MOS 中我们用到的 shell 命令是内置命令还是外部命令？请思考为什么 Linux 的 cd 命令是内部命令而不是外部命令？ **

我们用到的shell命令是外部命令，我们使用的cat和ls等都在user目录下有C语言文件。Linux下的cd指令没有对应的文件，使用时也不需要单独的创建一个子进程。cd 所做的是改变 shell 的 **PWD**。 因此倘若 cd 是一个外部命令，在进行时会新建一个子进程在子进程中运行cd指令，那么它改变的将会是子 shell 的 PWD，也不会向父 shell 返回任何东西。所以，当前 shell 的 PWD 就不会做任何改变。**所有能对当前 shell的环境作出改变的命令都必须是内部命令。** 

----



### Thinking 6.8

> **在你的 shell 中输入命令 ls.b | cat.b > motd。 **
>
> **• 请问你可以在你的 shell 中观察到几次 spawn ？分别对应哪个进程？ **
>
> **• 请问你可以在你的 shell 中观察到几次进程销毁？分别对应哪个进程？**

运行的结果如下：

![image-20230602225038560](C:\Users\HONOR HUNTER\AppData\Roaming\Typora\typora-user-images\image-20230602225038560.png)

- 应该有两次spawn，一次是解析ls.b文件一次是解析cat.b文件
- 在shell中可以观察到四次进程的销毁，最先销毁的是`[00003805] destroying 00003805`对应的应该是解析和运行ls.b的进程，即管道的输入进程；第二个被销毁的是`[00004006] destroying 00004006`应该是解析和运行cat.b的进程，即管道的输入进程；第三个被销毁的是`[00003004] destroying 00003004`对应的一个是管道进程；最后被销毁的`[00002803] destroying 00002803`是执行整条命令的子shell进程

----



## 实验难点

### Exercise 6.1

> **根据上述提示与代码中的注释，填写 user/lib/pipe.c 中的 pipe_read、 pipe_write、_pipe_is_closed 函数并通过 testpipe 的测试。 **

通过**_pipe_is_closed**函数判断管道是否关闭，若未关闭则持续从管道中写入和读出，**pipe_read**代码如下，**pipe_wirte**与其类似

```C
rbuf = (char *) vbuf;
p = (struct Pipe *)fd2data(fd);
i = 0;
while(1) {
    if (p->p_rpos == p->p_wpos) {
        if (_pipe_is_closed(fd, p)) {
            return i;
        } else {
            syscall_yield();
        }
    } else {
        if (i == n) {
            return i;
        }
        rbuf[i] = p->p_buf[p->p_rpos % BY2PIPE];
        i++;
        p->p_rpos++;
    }
}
```

-----



### Exercise 6.2 & 6.3

调换map顺序即可，具体原因在思考题部分做过解释

---



### Exercise 6.4

> **根据以上描述以及注释，补充完成 user/lib/spawn.c 中的 int spawn(char *prog, char \*\*argv)。 **

此函数的作用是读取一个可执行二进制文件，并新建一个子进程运行此可执行文件，具体步骤和fork函数比较相像，我们需要补全的部分为加载elf文件到子进程中，与load_icode函数比较接近

```C
size_t ph_off;
ELF_FOREACH_PHDR_OFF (ph_off, ehdr) {
    r = seek(fd, ph_off); //使文件指针偏移
    if (r < 0) {
        goto err1;
    }
    r = readn(fd, elfbuf, ehdr->e_phentsize); //读取文件内容
    if (r < 0) {
        goto err1;
    }
    Elf32_Phdr *ph = (Elf32_Phdr *)elfbuf;
    if (ph->p_type == PT_LOAD) {
        void *bin;

        r = read_map(fd, ph->p_offset, &bin);
        if (r < 0) {
            goto err1;
        }

        r = elf_load_seg(ph, bin, (elf_mapper_t)spawn_mapper, &child);
        if (r < 0) {
            goto err1;
        }
    }
}
```

-----



### Exercise 6.5

> **根据以上描述，补充完成 user/sh.c 中的 void parsecmd(char \*\*argv, int *rightpipe)。**

此函数作用是解析命令，当读到**"<"**、**">"**和**"|"**分别进行对应操作

-----



## 心得体会

最后一次实验难度不是很大，但发现自己lab5的代码出现bug导致了lab6测试一直通过不了，排查bug的过程很痛苦，但最后完成整个实验，看着自己修修补补一学期的小操作系统能够执行命令是一件十分开心的事。回顾一学期的实验课，虽然课上实验挂了不少，但感觉还是很有收获的，通过自己补写代码，加深了对理论课上一些知识的理解，可能到最后对整个MOS还不是特别了解，不过能看着MOS功能一点点变多也充满着乐趣。
