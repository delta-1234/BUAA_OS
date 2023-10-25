# Lab0实验报告

## 一、思考题

### Thinking 0.1

首先新建learnGit目录，并使用`git init`进行git的初始化，在*learnGit*目录下使用`touch REAME.txt`新建README.txt

- 执行`git status > Untracked.txt`后，*Untracked.txt*的文件内容如下：

  可以看到，因为没有使用`git add`指令追踪两个文件，所以两个文件都处于**untracked**状态

  ```shell
  git@21373221:~/learnGit (master)$ cat Untracked.txt 
  位于分支 master
  
  尚无提交
  
  未跟踪的文件:
    （使用 "git add <文件>..." 以包含要提交的内容）
          README.txt
          Untracked.txt
  
  提交为空，但是存在尚未跟踪的文件（使用 "git add" 建立跟踪）
  ```

- 使用vim在README.txt中添加内容，再使用`git add .`指令追踪文件，执行命令`git status > Stage.txt`后，Stage.txt的文件内容如下：

  可以看到，*README.txt*和*Untracked.txt*都被追踪了，处于**Staged**状态。*Stage.txt*文件因为是在重定向时创建的，所以未被添加到暂存区

  ```shell
  git@21373221:~/learnGit (master)$ cat Stage.txt 
  位于分支 master
  
  尚无提交
  
  要提交的变更：
    （使用 "git rm --cached <文件>..." 以取消暂存）
          新文件：   README.txt
          新文件：   Untracked.txt
  
  未跟踪的文件:
    （使用 "git add <文件>..." 以包含要提交的内容）
          Stage.txt
  ```

- 使用`git commit -m "21373221"`提交*README.txt*，再修改*README.txt*的内容，执行指令`git status > Modified.txt`，可以看到*Modified.txt*的文件内容如下：

  开始使用`git commit`指令后，*README.txt*被提交，处于**Unmodified**状态，之后在工作区修改了*README.txt*的内容，修改后的*README.txt*未被add到暂存区中，所以处于**Modified**状态。这种状态与开始**Untracked**状态不同，因为还有一份未修改的*README.txt*保存在git仓库中，可以使用`git restore`指令进行恢复，而一开始的**Untracked**状态一旦修改是无法从git仓库中恢复的。

  ```shell
  git@21373221:~/learnGit (master)$ cat Modified.txt 
  位于分支 master
  尚未暂存以备提交的变更：
    （使用 "git add <文件>..." 更新要提交的内容）
    （使用 "git restore <文件>..." 丢弃工作区的改动）
          修改：     README.txt
  
  未跟踪的文件:
    （使用 "git add <文件>..." 以包含要提交的内容）
          Modified.txt
          Stage.txt
  
  修改尚未加入提交（使用 "git add" 和/或 "git commit -a"）
  ```




-----



### Thinking 0.2

add the file对应的是`git add`指令

stage the file对应的是`git add`指令，即所有将工作区文件保存到暂存区的操作都可以使用`git add`来完成

commit即将暂存区的文件提交到本地git仓库的**HEAD**中，对应的是`git commit`指令



----



### Thinking 0.3

1. 代码文件 print.c 被错误删除时，应当使用什么命令将其恢复？

   > 如果删除后未使用`git add`提交到暂存区，可以使用`git checkout print.c`或者`git restore print.c`进行恢复

2. 代码文件 print.c 被错误删除后，执行了 git rm print.c 命令，此时应当

   使用什么命令将其恢复？

   > 如果print.c未被提交过，则暂存区和**HEAD**中都没有print.c，无法恢复；如果被提交过，则可从**HEAD**中恢复，使用`git reset HEAD print.c`指令将其恢复到暂存区，再使用上一问方法即可恢复到工作区


3. 无关文件 hello.txt 已经被添加到暂存区时，如何在不删除此文件的前提下

   将其移出暂存区？

   > 使用`git rm hello.txt`即可将其移除暂存区



----



### Thinking 0.4

1. 进行三次提交后，执行`git log`的结果如下：

```shell
git@21373221:~/learnGit (master)$ git log
commit f27d16229599552553266f18ec989d4866f08e2a (HEAD -> master)
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:36:37 2023 +0800

    3

commit 180af5a9e8dcfb23227c6c0f86ad235235e2e637
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:36:07 2023 +0800

    2

commit 6818308c515b18d21ba674395350fdbaa18b3d4f
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:35:32 2023 +0800

    1
```

2. 执行`git reset --hard HEAD^`后，再此执行`git log`结果如下：

   可以观察到**HEAD**的位置指向了提交信息为2的提交，提交信息为3的提交被退回了

```shell
git@21373221:~/learnGit (master)$ git reset --hard HEAD^
HEAD 现在位于 180af5a 2
git@21373221:~/learnGit (master)$ git log
commit 180af5a9e8dcfb23227c6c0f86ad235235e2e637 (HEAD -> master)
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:36:07 2023 +0800

    2

commit 6818308c515b18d21ba674395350fdbaa18b3d4f
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:35:32 2023 +0800

    1
```

3. 执行`git reset --hard 6818308c515b18d21ba674395350fdbaa18b3d4f`后，再执行`git log`的结果如下：

   此时**HEAD**指向了提交信息为1的提交，即回退到了1版本

```shell
git@21373221:~/learnGit (master)$ git reset --hard 6818308c515b18d21ba674395350fdbaa18b3d4f
HEAD 现在位于 6818308 1
git@21373221:~/learnGit (master)$ git log
commit 6818308c515b18d21ba674395350fdbaa18b3d4f (HEAD -> master)
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:35:32 2023 +0800

    1
```

4. 想回到新版本即3版本，执行`git reset --hard f27d16229599552553266f18ec989d4866f08e2a`，再执行`git log`结果如下：

   可以看到**HEAD**指向回到了初始，即又返回了新版本3

```shell
git@21373221:~/learnGit (master)$ git reset --hard f27d16229599552553266f18ec989d4866f08e2a
HEAD 现在位于 f27d162 3
git@21373221:~/learnGit (master)$ git log 
commit f27d16229599552553266f18ec989d4866f08e2a (HEAD -> master)
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:36:37 2023 +0800

    3

commit 180af5a9e8dcfb23227c6c0f86ad235235e2e637
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:36:07 2023 +0800

    2

commit 6818308c515b18d21ba674395350fdbaa18b3d4f
Author: 邓韬 <21373221@buaa.edu.cn>
Date:   Sat Mar 4 20:35:32 2023 +0800

    1
```

在执行上述操作的过程中我发现，如果使用`git reset --hard`指令退回版本，工作区的未被提交的修改都会被覆盖，比如在执行第2步前，将*README.txt*内容修改为Testing 4，但并未提交，而在执行第2步之后，*README.txt*中内容会被强制修改为Testing 2并且无法回到之前的内容。



----



### Thinking 0.5

- 执行`echo first`，终端输出first

- 执行`echo second > output.txt`，终端无输出，使用`cat output.txt`输出second

- 执行`echo third > output.txt`，终端无输出，使用`cat output.txt`输出third

- 执行`echo forth >> output.txt`，终端无输出，使用`cat output.txt`输出为

  > third
  >
  > forth

从以上结果中可以得知，使用`>`可以使输出重定向到文件中，若文件不存在则创建，若文件存在则覆盖文件中的内容。使用`>>`也可以将输出重定向到文件中，若文件不存在则创建，若文件存在则将输出内容添加在原内容后。



----



### Thinking 0.6

command文件内容如下：

```sh
#!/bin/bash
echo 'echo Shell Start...
echo set a=1 
a=1 
echo set b 
b=2
echo set c = a+b 
c=$[$a+$b]
echo c = $c
echo save c to ./file1 
echo $c>filel
echo save b to./file2 
echo $b>file2
echo save a to ./file3 
echo $a>file3
echo save filel file2 file3 to file4 
cat filel>file4 
cat file2>>file4 
cat file3>>file4
echo save file4 to ./result 
cat file4>>result' > test
```

执行命令`bash command`后，test文件内容如下：

```sh
echo Shell Start...
echo set a=1 
a=1 
echo set b 
b=2
echo set c = a+b 
c=$[$a+$b]
echo c = $c
echo save c to ./file1 
echo $c>filel
echo save b to./file2 
echo $b>file2
echo save a to ./file3 
echo $a>file3
echo save filel file2 file3 to file4 
cat filel>file4 
cat file2>>file4 
cat file3>>file4
echo save file4 to ./result 
cat file4>>result
```

使用`bash test`后终端输出如下：

```sh
Shell Start...
set a=1
set b
set c = a+b
c = 3
save c to ./file1
save b to./file2
save a to ./file3
save filel file2 file3 to file4
save file4 to ./result
```

result文件内容如下：

```sh
3
2
1
```

**结果分析**

结合test中的内容，首先创建了a，b两个变量，并将a赋值为1，b赋值为2，a和b向加得到变量c。第一步将c的值即3保存到file1中，变量b的值保存到file2中，变量a的值保存到file3中，第二步将file1、file2、file3中内容加在file4中，最后将file4内容追加在result文件中，所以result中其实就依次存储了c、b、a的值，即3、2、1

**思考**

- `echo echo Shell Start` 与 ``echo `echo Shell Start` ``效果有区别，前者输出**echo Shell Start**后者输出**Shell Start**，添加反引号后，显示的不是直接内容，而是反引号中命令的结果
- `echo echo $c>file1` 与 ``echo `echo $c>file1` ``效果有区别，如c的值为3，前者在终端不输出，在file1中显示**echo 3**；后者在终端也不输出，在file1中显示**3**



## 二、实验难点分析

###  Exercise 0.1

1. 主要考察C语言的基础使用，难度不大，使用循环进行判断即可
2. 难点在于Makefile编写和gcc指令的使用，在Makefile中使用`gcc -o palindrome.c palindrome`即可
3. 考察shell脚本编写和sed指令使用，在脚本中先使用`touch $2`创建指定文件名的文件，再使用`sed -n '8p;32p;128p;512p;1024p' $1 > $2`进行文本的替换
4. 使用cp指令进行复制即可



----



### Exercise 0.2

1. 考察shell脚本中循环和条件语句的使用，使用`rm -f`删除文件并使用`mv`进行重命名即可，注意在循环中要使用`let a=a+1`使a自增，不然会陷入死循环



-----



### Exercise 0.3

1. 考察shell脚本编写和grep、awk指令使用，先使用`touch $3`创建指定文件，再使用`grep -n $2 $1 | awk -F: '{print $1}' > $3`，grep -n可以得到文件中字符串所在行数，awk -F可结果以`:`分割并输出第一项，使用管道进行连接可以省去重定向中间文件的麻烦



----



### Exercise 0.4

1. 考察shell脚本的编写和sed指令的使用，使用`sed -i "s/$2/$3/g" $1`进行字符串的替换即可，-i表示编辑文本，不能省略，否则只输出到终端而不修改，g必须添加进行整行替换，否则只替换此行第一个匹配字符串
2. 考察Makefile编写，上机时没有很明白，于是只编写了csc中的Makefile，使用`gcc code/fibo.c -I include -o code/fibo.o`方式进行编译，最后再`gcc code/fibo.c code/main.c -I include -o fibo`生成fibo。虽然通过了评测，但没有使用到code中的Makefile不是很好的方法，课后经过思考，应该将csc/code中Makefile编写为如下内容：

```makefile
all: main.c fibo.c
	gcc -c fibo.c -I ../include
	gcc -c main.c -I ../include
	gcc -o ../fibo fibo.o main.o
clean: 
	rm -f *.o
```

csc中Makefile应该编写为：

```makefile
all: code
	$(MAKE) -C code
clean:
	$(MAKE) clean -C code
```



## 三、实验体会

上机的时间还是比较紧张，虽然提前进行了一些预习，但在上机的使用过程中还不是很熟练，对应Linux的命令和git操作只是略微进行了了解，导致在上机时需要不断翻阅参考资料降低了实验速度。最后的Exercise 0.4虽然通过比较取巧的方式通过了评测，但没有真正理解到Makefile跨目录编写的方法，课下还是需要重新思考，并对Linux命令和git操作多加练习。
