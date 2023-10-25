# Lab5实验报告

## 思考题

## Thinking 5.1

> **如果通过 kseg0 读写设备，那么对于设备的写入会缓存到 Cache 中。这是一种错误的行为，在实际编写代码的时候这么做会引发不可预知的问题。请思考：这么做这会引发什么问题？对于不同种类的设备（如我们提到的串口设备和 IDE 磁盘）的操作会有差异吗？可以从缓存的性质和缓存更新的策略来考虑。 **

缓存机制是为了提高效率而设计的，具体体现为数据发生改变时并不立即写入内存，而是在cache发生替换时才写入。若写入kseg0部分，数据先缓存在cache中，如果此时发生中断则不会被真正写入内存，那么设备得到的输入就不是实际需要的内容，将会得到不确定的结果。

存在差异，串口设备的读写频率远高于IDE磁盘并且要求需要有更快的响应速度，在缓存更新的策略上，串口设备应该设置更高的缓存刷新频率。

-----



### Thinking 5.2

> **查找代码中的相关定义，试回答一个磁盘块中最多能存储多少个文件控制块？一个目录下最多能有多少个文件？我们的文件系统支持的单个文件最大为多大？ **

一个磁盘块大小为`BY2BLK`为4096B，一个文件控制块大小为256B，所以一个磁盘块最多能存储16个文件控制块。一个目录中直接文件指针和间接文件指针对应的磁盘块最多有1024个，最多有`1024*16=16384`个文件。单个文件最大为`1024*4096B = 4MB`

----



### Thinking 5.3

> **请思考，在满足磁盘块缓存的设计的前提下，我们实验使用的内核支持的最大磁盘大小是多少？** 

DISKMAX为`0x40000000=2^30`，所以支持的最大磁盘大小为1GB

---



### Thinking 5.4

> **在本实验中，fs/serv.h、user/include/fs.h 等文件中出现了许多宏定义， 试列举你认为较为重要的宏定义，同时进行解释，并描述其主要应用之处。**

**fs/serv.h**：

- BY2SECT：每个磁盘扇区的大小单位为Byte，实际为512
- SECT2BLK：将扇区号转换为磁盘块号，实际上为4096/512 = 16
- DISKMAP：磁盘缓存映射的开始地址，主要应用为计算地址
- DISKMAX：磁盘缓存的最大值

**user/include/fs.h**：

- BY2BLK：每个磁盘块大小，单位为Byte，与页大小相同，都为4096B
- BIT2BLK：每个磁盘大小，单位为Bit，为BY2BLK*8
- MAXNAMELEN：文件名最大长度，128字节，包括最后的'\0'
- MAXPATHLEN：路径最大长度，1024字节，包括最后的'\0'
- NDIRECT：文件控制块中直接指针数量，应用于文件内容的查找
- NINDIRECT：文件控制块中所有指针数量，包括直接指针和间接指针指向的磁盘块中的指针
- MAXFILESIZE：文件的最大大小，单位Byte，实际上为4MB
- BY2FILE：文件控制块的大小，单位Byte，实际上为256B
- FILE2BLK：每个磁盘块中文件控制块最大数量，实际为4096/256=16
- FTYPE_REG：文件类型，常规文件
- FTYPE_DIR：文件类型，目录文件
- FS_MAGIC：Super块中的魔数

----



### Thinking 5.5

> **在 Lab4“系统调用与 fork”的实验中我们实现了极为重要的 fork 函数。那么 fork 前后的父子进程是否会共享文件描述符和定位指针呢？请在完成上述练习的基础上 编写一个程序进行验证。 **

我在用户态下编写了一个简单程序

```C
int main() {
    fs_init();
    // fs_check();
    int r;
    struct File *f;
    debugf("My test begin\n");
    if ((r = file_open("/newmotd", &f)) < 0) {
        user_panic("file_open /newmotd: %d", r);
    }

    debugf("file_open is good\n");
    int child = fork();
    if (child == 0) {
        if ((r = file_set_size(f, 0)) < 0) {
            user_panic("file_set_size: %e", r);
        }
        debugf("child can get file\n");
    } else {
        if ((r = file_set_size(f, 0)) < 0) {
            user_panic("file_set_size: %e", r);
        }
        debugf("father can get file\n");
    }
    return 0;
}
```

运行的结果如下：

![image-20230519003922394](C:\Users\HONOR HUNTER\AppData\Roaming\Typora\typora-user-images\image-20230519003922394.png)

说明子进程和父进程共享文件文件描述符和定位指针

----



### Thinking 5.6

> **请解释 File, Fd, Filefd 结构体及其各个域的作用。比如各个结构体会在哪些过程中被使用，是否对应磁盘上的物理实体还是单纯的内存数据等。说明形式自定，要求简洁明了，可大致勾勒出文件系统数据结构与物理实体的对应关系与设计框架。**

File结构体其实就为文件控制块，用于描述和管理文件，能够对应到文件在磁盘上的物理实体，记录了文件的分布及文件之间的关系

```C
struct File {
	char f_name[MAXNAMELEN]; // 文件名
	uint32_t f_size;	 // 文件大小
	uint32_t f_type;	 // 文件类型
	uint32_t f_direct[NDIRECT]; //指向磁盘块的直接指针
	uint32_t f_indirect; //间接指针
	struct File *f_dir; //指向父目录
	char f_pad[BY2FILE - MAXNAMELEN - (3 + NDIRECT) * 4 - sizeof(void *)];
} __attribute__((aligned(4), packed));
```

Fd结构体记录文件的基本信息，包括文件的状态和权限，不对应磁盘上物理实体

```C
struct Fd {
	u_int fd_dev_id; //文件存储外设的id
	u_int fd_offset; //读写的偏移量
	u_int fd_omode; //文件的打开方式，只读、只写和读写
};
```



## 实验难点

### Exercise 5.1

> **请根据 kern/syscall_all.c 中的说明，完成 sys_write_dev 函数以及 sys_read_dev 函数的实现。 **
>
> **编写这两个系统调用时需要注意物理地址与内核虚拟地址之间的转换。**
>
> **同时还要检查物理地址的有效性，在实验中允许访问的地址范围为: **
>
> **console:[0x10000000, 0x10000020), disk:[0x13000000, 0x13004200), rtc:[0x15000000, 0x15000200)，当出现越界时，应返回指定的错误码。**

这两个函数编写时注意范围判断为左右需要等于，在使用memcpy时需要将pa加上KSEG1转换为虚拟地址

-----



### Exercise 5.2

> **在 user/lib/syscall_lib.c 中完成用户态的相应系统调用的接口。**

使用msyscall进行系统调用即可

----



### Exercise 5.3

> **参考以上展示的内核态驱动，使用系统调用完成 fs/ide.c 中的 ide_write 函数，以及 ide_read 函数，实现对磁盘的读写操作。**

按照提示向对应的物理地址读写磁盘即可，注意在ide_write函数中需要先将数据写入缓冲区再进行后续的操作

----



### Exercise 5.4

> **文件系统需要负责维护磁盘块的申请和释放，在回收一个磁盘块时，需要更改位图中的标志位。如果要将一个磁盘块设置为 free，只需要将位图中对应位的值设置 为 1 即可。请完成 fs/fs.c 中的 free_block 函数，实现这一功能。同时思考为什么参数 blockno 的值不能为 0？**

0号block是作引导扇区和分区表不能进行修改，所以blockno不能为0

-----



### Exercise 5.5

> **参照文件系统的设计，完成 fsformat.c 中的 create_file 函数，并阅读 write_directory 函数（代码已在源文件中给出，不作为考查点），实现将一个文件或指定目录下的文件按照目录结构写入到 target/fs.img 的功能。**

```C
if (i < NDIRECT) {
    bno = dirf->f_direct[i];
} else {
    bno = ((int *)disk[dirf->f_indirect].data)[i];
}
```

当需要读取的磁盘块大于10时，需要使用间接指针访问磁盘再从磁盘中读取到指针，注意磁盘块的前10个指针无效

-----



### Exercise 5.6

> **fs/fs.c 中的 diskaddr 函数用来计算指定磁盘块对应的虚存地址。完成 diskaddr 函数，根据一个块的序号 (block number)，计算这一磁盘块对应的虚存的起始地址。（提示：fs/serv.h 中的宏 DISKMAP 和 DISKMAX 定义了磁盘映射虚存的地址空间）。**

直接使用`return (void *)(DISKMAP + blockno * BY2BLK)`即可

----



### Exercise 5.7

> **实现 map_block 函数，检查指定的磁盘块是否已经映射到内存，如果没有，分配一页内存来保存磁盘上的数据。相应地，完成 unmap_block 函数，用于解除磁盘块和物理内存之间的映射关系，回收内存。（提示：注意磁盘虚拟内存地址空间和磁盘块之间的对应关系）。**

按照要求使用`block_is_mapped`、`block_is_free`和`block_is_dirty`进行判断，再使用相应的系统调用实现分配内存的分配和解除映射

----



### Exercise 5.8

> **补全 dir_lookup 函数，查找某个目录下是否存在指定的文件。 （使用 file_get_block 函数）**

使用`file_get_block(dir, i, &blk)`来获得指定的磁盘块，注意在找到对应的文件之后需要返回0，没有找到则需要返回`-E_NOT_FOUND`

----



### Exercise 5.9

> **请完成 user/lib/file.c 中的 open 函数。（提示：若成功打开文件，则该函数返回文件描述符的编号）。 **

按照提示进行，注意Filefd和Fd两个类型的结构体指针的转换，最后使用fd2num(fd)返回文件描述符的编号

---



### Exercise 5.10

> **参考 user/lib/fd.c 中的 write 函数，完成 read 函数**

仿照write函数的编写，将函数替换为dev->dev_read，并且注意读出的buf并不是C语言要求的字符串，我们需要在其最后加上'\0'

----



### Exercise 5.11 & 5.12 & 5.13

此三个函数共同完成了删除指定路径的 文件的功能，用户能够调用的是remove函数，而 serve_remove和fsipc_remove分别是在文件系统和用户态下的实现

----



## 心得体会

这部分实验代码量比较大，涉及到的文件也比较多，要理解各个文件之间的关系比较困难，虽然补全代码比较容易，但对代码整体的了解还是一知半解。即使到了lab5实际我自身感觉对整个操作系统的了解度还不是很高，只是有了一个整体的模糊印象，看着提示补全代码让我有一种盲人摸象的感觉，即使OS上机即将结束，对GXemul调试还是不太熟练，但通过使用打印流的方式也掌握了比较系统的调试方法
