## InterProcess Communication

### 1.管道

**管道（pipe）**是UNIX系统IPC的最古老形式，其局限性在于

* 半双工（即数据只能在一个方向上流动）
* 管道只能在具有公共祖先的两个进程之间使用。通常，一个管道由一个进程创建，在此进程`fork`之后，这个管道就能在父进程和子进程之间使用了

> PS：FIFO没有第二种局限性，UNIX套接字没有这两这个局限性

管道的函数：

	#include <unistd.h>
	int pipe(int fd[2]);	//成功返回0，出错返回-1

`fd[0]`为读打开，`fd1[1]`为写而打开

当读一个写端已被关闭的管道时，在所有数据被读取后，`read`返回0，表示文件结束

如果写一个读端已被关闭的管道，则产生信号`SIGPIPE`，如果忽略该信号或者捕捉该信号并从其处理程序返回，则`write`返回-1，`errno`设置为`EPIPE`

**`popen`和`pclose`**

`popen`的操作是：创建一个管道，`fork`一个子进程

`pclose`的操作是：关闭未使用的管道端，执行一个shell运行命令，然后等待命令终止

**函数原型**

	#include <stdio.h>
	FILE *fopen(const char *cmdstring, const char *type);	//成功返回文件指针，否则返回NULL
	int fclose(FILE *fp);	//成功返回cmdstring的终止状态，出错返回-1

**实现shell的`ls`命令的一个简单例子**
	
	#include "apue.h"

	int main()
	{
		FILE *fp = NULL;
		if ((fp = popen("/bin/ls", "r"))== NULL) {
			err_sys("popen error");
		} else {
			char line[MAXLINE];
			fread(line, sizeof(char), sizeof(line), fp);
			write(STDOUT_FILENO, line, (int)strlen(line));
			pclose(fp);
		}
		exit(0);
	}

### 2.协同进程

`popen`只提供连接到另一个进程的标准输入或标准输出的一个单向管道，**协同进程**则有连接到另一个进程的两个管道：一个连接到标准输入（fd[0]），一个则来自标准输出（fd[1]）。

### 3.FIFO

创建FIFO类似创建文件，FIFO的路径名存在于文件系统中

	#include <sys/stat.h>
	int mkfifo(const char *path, mode_t mode);
	int mkfifoat(int fd, const char *path, mode_t mode);
	//两个函数成功返回0，否则返回-1

**FIFO的用途**

* shell命令使用FIFO将数据从一条管道传送到另一条时，无需创建中间临时文件
* 客户进程-服务器进程应用程序中，FIFO用作汇聚点，在客户进程和服务器进程二者之间传递数据

### 3.XSI IPC

	#include <sys/ipc.h>
	key_t ftok(const char *path, int id);

`key_t`包含于`<sys/types.h>`中，定义为长整型

使客户进程和服务器进程在同一IPC结构上汇聚

> 通过指定键为`IPC_PRIVATE`创建一个新的IPC结构，把返回的标识符存放在某处以便客户进程取用

### 4.消息队列

`msgget`用于创建一个新队列或打开一个现有队列，`msgsnd`将新消息添加到队列尾端，`msgrcv`用于从队列中取消息。这里不一定要以先进先出次序取消息，也可以按消息的类型字段取消息

	#include <sys/msg.h>
	int msgget(key_t key, int flag); //成功返回消息队列ID，否则返回-1
	int msgctl(int msqid, int cmd, struct msqid_ds *buf); //成功返回0，否则返回-1
	int msgsnd(int msqid, const void *ptr, size_t nbytes, int flag); //成功返回0，否则返回-1
	int msgrcv(int msqid, void *ptr, size_t nbytes, long type, int flag); //成功返回消息数据部分的长度，否则返回-1

这里`msgctl`的`cmd`参数的取值有

> `IPC_STAT`: 取此队列的`msqid_ds`结构，并存放到`buf`中

> `IPC_RMID`: 从系统中删除该消息队列以及仍在该队列中的所有数据

> `msgsnd`里`ptr`指向一个长整型数，`nbytes`为0则无消息数据。`msgrcv`类似，而多出的`type`则是可以用于标记队列中特点的某个消息

### 5.信号量

**信号量（semaphore）**与管道，消息队列不同，它是一个计数器，用于为多个进程提供对共享数据的对象的访问

	#include <sys/sem.h>
	int semget(key_t key, int nsems, int flag); //成功返回信号量ID，否则返回-1
	int semop(int semid, struct sembuf semoparray[], size_t nops_; //成功返回0，否则-1

`segget`中的`nsems`参数表示信号数，`semop`中的nops指对信号的操作，正值代表生产资源，负值代表获取资源

