# Lab1实验报告

## 思考题

### Thinking 1.1

创建一个最简单的C语言程序hello.c，使用`gcc -c hello.c`进行编译但不链接，得到了hello.o文件，使用`objdump -DS hello.o`输出反汇编，因为使用的是x86的原生工具链，所以反编译出的是x86汇编，main函数结果如下：

```assembly
hello.o：     文件格式 elf64-x86-64


Disassembly of section .text:

0000000000000000 <main>:
   0:   f3 0f 1e fa             endbr64 
   4:   55                      push   %rbp
   5:   48 89 e5                mov    %rsp,%rbp
   8:   48 8d 05 00 00 00 00    lea    0x0(%rip),%rax        # f <main+0xf>
   f:   48 89 c7                mov    %rax,%rdi
  12:   e8 00 00 00 00          call   17 <main+0x17>
  17:   b8 00 00 00 00          mov    $0x0,%eax
  1c:   5d                      pop    %rbp
  1d:   c3                      ret
```

相对的如果使用`mips-linux-gnu-gcc -c hello.c`和`mips-linux-gnu-objdump -DS hello.o`输出，结果如下：

```assembly
hello.o：     文件格式 elf32-tradbigmips


Disassembly of section .text:

00000000 <main>:
   0:   27bdffe0        addiu   sp,sp,-32
   4:   afbf001c        sw      ra,28(sp)
   8:   afbe0018        sw      s8,24(sp)
   c:   03a0f025        move    s8,sp
  10:   3c1c0000        lui     gp,0x0
  14:   279c0000        addiu   gp,gp,0
  18:   afbc0010        sw      gp,16(sp)
  1c:   3c020000        lui     v0,0x0
  20:   24440000        addiu   a0,v0,0
  24:   8f820000        lw      v0,0(gp)
  28:   0040c825        move    t9,v0
  2c:   0320f809        jalr    t9
  30:   00000000        nop
  34:   8fdc0010        lw      gp,16(s8)
  38:   00001025        move    v0,zero
  3c:   03c0e825        move    sp,s8
  40:   8fbf001c        lw      ra,28(sp)
  44:   8fbe0018        lw      s8,24(sp)
  48:   27bd0020        addiu   sp,sp,32
  4c:   03e00008        jr      ra
  50:   00000000        nop
        ...
```

可以看出mips汇编相比起x86复杂许多，但可以看,出`jalr t9`和`call`两条指令对应的就是printf函数，其中call的地址我们可以看到为0x0000，而t9中所含内容我们可以在`lw v0,0(gp)`和`move t9,v0`这两条指令中看出，进行前面操作后v0中所存值也为0x0000，这都表明printf此时还没没有被加载到程序中

分别使用`readelf -a hello.o`和`mips-linux-gnu-readelf -a hello.o`查看elf文件，可以发现x86为**小端序**而mips为**大端序**，二者的入口地址都为0x0并且都无程序头，这是因为printf函数没有实现，两个文件都只是**可重定位文件（REL）**

使用`gcc -o hello hello.c`进行编译链接，再使用`objdump -DS hello`进行反汇编，结果如下：

```assembly
hello：     文件格式 elf64-x86-64


Disassembly of section .interp:
...
0000000000001149 <main>:
    1149:       f3 0f 1e fa             endbr64 
    114d:       55                      push   %rbp
    114e:       48 89 e5                mov    %rsp,%rbp
    1151:       48 8d 05 ac 0e 00 00    lea    0xeac(%rip),%rax        # 2004 <_IO_stdin_used+0x4>
    1158:       48 89 c7                mov    %rax,%rdi
    115b:       e8 f0 fe ff ff          call   1050 <puts@plt>
    1160:       b8 00 00 00 00          mov    $0x0,%eax
    1165:       5d                      pop    %rbp
    1166:       c3                      ret 
...
```

可以看到call这一指令后被加上了正确的地址，同样使用`readelf -a hello`进行查看，发现入口点地址变为了0x1060，但并不是可执行文件，而是**共享对象文件（DYN）**

相对的加上前缀后`mips-linux-gnu`重复操作，结果如下：

```assembly
hello：     文件格式 elf32-tradbigmips


Disassembly of section .interp:
...
004006e0 <main>:
  4006e0:       27bdffe0        addiu   sp,sp,-32
  4006e4:       afbf001c        sw      ra,28(sp)
  4006e8:       afbe0018        sw      s8,24(sp)
  4006ec:       03a0f025        move    s8,sp
  4006f0:       3c1c0042        lui     gp,0x42
  4006f4:       279c9010        addiu   gp,gp,-28656
  4006f8:       afbc0010        sw      gp,16(sp)
  4006fc:       3c020040        lui     v0,0x40
  400700:       24440830        addiu   a0,v0,2096
  400704:       8f828030        lw      v0,-32720(gp)
  400708:       0040c825        move    t9,v0
  40070c:       0320f809        jalr    t9
  400710:       00000000        nop
  400714:       8fdc0010        lw      gp,16(s8)
  400718:       00001025        move    v0,zero
  40071c:       03c0e825        move    sp,s8
  400720:       8fbf001c        lw      ra,28(sp)
  400724:       8fbe0018        lw      s8,24(sp)
  400728:       27bd0020        addiu   sp,sp,32
  40072c:       03e00008        jr      ra
  400730:       00000000        nop
        ...
```

经过lw指令后v0的具体值我们不得而知，但一定不会是之前的0x0，所以jalr也能跳转到正确的地址，接着使用readelf指令后我们可以看到入口地址变为了0x400540，elf文件类型是**可执行文件（EXEC）**

向objdump传入的两个参数D和S可以使用man指令查到，D表示对二进制文件所有部分进行反汇编，去掉这个参数只保留S的话只会输出main函数部分的反汇编。S表示的尽可能输出反汇编源码，当编译时指定了-g调试参数时比较明显，本例子中去掉S无明显变化

----

### Thinking 1.2

- 先进入到tools/readelf目录下，执行命令`./readelf ~/21373221/target/mos`即可解析我们生成的ELF文件，输出的节头地址如下：

  ```
  0:0x0
  1:0x80010000
  2:0x80011d10
  3:0x80011d28
  4:0x80011d40
  5:0x0
  6:0x0
  7:0x0
  8:0x0
  9:0x0
  10:0x0
  11:0x0
  12:0x0
  13:0x0
  14:0x0
  15:0x0
  16:0x0
  ```

  结果与使用`readelf -S mos`得到的正确结果相同

- 如果使用`./readelf readelf`会发现没有任何结果，通过使用`readelf -h`查看*readelf*和*hello*的区别，可以很容易发现，*readelf*为**共享对象（DYN）**文件，而*hello*为**可执行（EXEC）**文件，由此我们可以推断出我们编写的readelf程序无法解析**DYN**类型的elf文件

  阅读Makefile可以发现，*hello*的生成比readelf多了三个参数`-m32 -static -g`，如果我们去除这三个参数再再次生成*hello*文件可以发现*hello*也变成了**DYN**文件，同样无法用我们编写的readelf程序解析；这三个参数中的`-static`表示静态链接，默认情况下为动态链接，这使得生成的elf文件为**EXEC**类型而不是**DYN**类型，如果使用`gcc hello.c -o hello -static`进行编译链接，可以发现生成的是EXEC类型，可以被解析

----

### Thinking 1.3

我们实验中的操作系统是在GXemul中运行的，Gxemul已经提供了bootload。MIPS体系结构上电的时候，启动地址为0xBFC00000，将地址高三位清零后得到0x1FC00000，此地址即在kseg1中开始执行bootload。实验中bootload在stage1阶段会初始化硬件设备，之后进入stage2阶段，我们的实验中GXemul能直接将elf格式的内核加载到内存中，而我们编写的Linker Script能保证内核被放在正确的位置上，并使得内核入口设置为_start函数，这样就保证了内核入口被正确跳转到

---



## 难点分析

### Exercise 1.1

readelf.c中第一部分我们要做的是得到节头表的地址，节头表的表项数和节头表的表项大小，具体做法如下：

```C
sh_table = binary + ehdr->e_shoff;//binary为elf文件头地址，e_shoff即节头表相对文件头偏移量
sh_entry_count = ehdr->e_shnum;
sh_entry_size = ehdr->e_shentsize;
```

第二部分我们要遍历节头表的每个表项输出每个表项的地址，难点就在于对C语言指针的理解，C语言指针其实就是地址，指针的类型是对地址的解释，所以先将**void ***类型的**sh_table**强制转换为**Elf32_Shdr ***类型，加上偏移i就可以得到此表项的地址，具体代码如下：

```C
shdr = (Elf32_Shdr *)sh_table;
addr = (shdr + i)->sh_addr;
```

----

### Exercise 1.2

查看内存布局图，可以很容易发现Kernel Text的起始地址为0x80010000，所以先将**"."**设置为0x80010000，**.text**、**.data**和**.bss**依次在其后定义即可

----

### Exercise 1.3

通过内存布局图我们可以发现栈指针的地址为0x80400000，所以只需要使用因为允许使用1号寄存器，所以可以使用`li sp, 0x80400000`指令，如果不能使用1号寄存器则需要用`lui sp,0x8040`

最后使用jal或者j（因为不需要返回）跳转到**mips_init**即可

----

### Exercise 1.4

printk函数的编写主要考察C语言的使用，第一部分需要找到下一个**%**，使用while循环搜索即可，具体的代码如下：

```c
while (*fmt != '%' && *fmt != '\0') {
	out(data, fmt, 1);
	fmt++;
}
```

之后的部分进行**if-else**判断即可，分别解析**format flag**、**width**和**length**，最后补全switch解析**specifier**，注意print_num函数传入的参数中num为绝对值，通过neg_flag区分正负，具体代码如下：

```C
neg_flag = (num < 0) ? 1 : 0;
num = (num < 0) ? -num : num;
print_num(out, data, num, 10, neg_flag, width, ladjust, padc, 0);
```

----



## 实验体会

课下部分习题比较简单，但实验指导书内容比较丰富。elf文件理解起来比较困难，对其各种类型和其中所含信息还处于一知半解状态。lab1中所给出的实验代码只是研究了习题相关部分，而且大部分习题都是根据提示完成的，对于其深刻内涵没有过多的了解，实验内核的大部分代码没有查看过，对实验的操作系统没有一个整体的认识

课下习题都是使用提示的方式进行测试的，不太会使用GXemul进行调试，虽然知道如何进行调试但对调试的输出信息不知道其意义，教程里面只是说明了GXemul的基础使用，但要真正将其用于debug还需要摸索
