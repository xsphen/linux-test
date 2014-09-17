## Chapter 11 *Thread*

### Concept

一个进程可以看成只有一个控制线程，这时一个进程在某一时刻只能做一件事情。在多个控制线程的情况下，在程序设计时就可以把进程设计成在某一时刻能够做不止一件事情，每个线程处理各自单独的任务。

* 通过为每种事件类型分配单独的处理线程，可以简化处理异步事件的代码
* 多个线程可以访问相同的存储地址空间和文件描述符
* 把一个进程的任务串行化，相互独立的任务就可以通过线程交叉进行，此时只需为每个任务分配一个单独的线程。问题的分解可以提高程序的吞吐量
* 交互的程序可以通过使用多线程来改善响应时间，比如多线程可以把程序中处理用户输入输出的部分与其他部分分开

每个线程包含有表示执行环境所必须的信息，包括线程ID，寄存器，栈，调度优先级和策略，errno变量以及线程私有数据。

一个进程的所有信息对该进程的所有线程都是共享的，包括可执行程序的代码，程序的全局内存和堆内存，栈以及文件描述符。

### 线程标识

线程ID，是一个`pthread_t`数据类型，该类型是一个结构。

线程可以通过`pthread_self`获得自己的线程ID

	#include <pthread.h>
	pthread_t pthread_self();

由于无法把`pthread_t`类型当作整数处理，所以判断线程ID是否相等需要用到一个函数：

	#include <pthread.h>
	int pthread_equal(pthread_t tid1, pthread_t tid2);//相等返回非0，不等返回0

### 线程创建

在POSIX线程中，通过如下方法创建进城：

	#include <pthread.h>
	int pthread_create(pthread_t *restrict tidp,
			const pthread_attr_t *restrict attr,
			void *(*start_rtn)(void *), void *restrict arg); //成功返回0，否则返回错误编号

第一个参数用来存储线程ID，第二个参数控制线程属性，第三个参数是线程从`start_rtn`函数的地址开始运行，第四个参数是`start_rtn`函数的指针参数

### 线程终止

单个线程在不终止整个进程的情况下的3种退出方式

* 线程从启动例程中返回，返回值是线程的退出码
* 线程被同一进程中的其他线程取消
* 线程调用`pthread_exit`

**方法一：**

	#include <pthread.h>
	void pthread_exit(void *rval_ptr);
	int pthread_join(pthread_t thread, void **rval_ptr);//成功返回0，否则返回错误编码

`pthread_exit`用来终止线程，若线程从启动例程中返回，则`rval_ptr`包含返回码，如果线程被取消，则`rval_ptr`所指的内存单元就被设置为`PTHREAD_CANCELED`

`pthread_join`用来获得其他线程退出的`rval_ptr`内容

如果不关心线程的退出码，把参数`rval_ptr`设为`NULL`，这样`pthread_join`调用时也不再获得该线程的退出状态

**方法二：**

	#include <pthread.h>
	int pthread_cancel(pthread_t tid); //成功返回0，否则返回错误编码

值得注意的是，`pthread_cancel`只是提出一个“取消建议”，最终是否取消取决于线程ID为`tid`

**方法三：**

	#include <pthread.h>
	void pthread_cleanup_push(void (*rtn)(void *), void *arg);
	void pthread_cleanup_pop(int execute);

跟进程的`atexit`函数一样，线程也拥有可以安排它退出是需要调用的函数，该函数称之为*线程清理处理程序（thread cleanup handler）*，注意该清理函数的执行顺序跟注册时相反

rtn被执行的条件有：

* 线程调用`pthread_exit`
* 线程相应取消请求
* execute != 0的`pthread_cleanup_pop`调用

注：当线程从启动线程中返回时，rtn不会被执行！

**方法三：**

	#include <pthread.h>
	int pthread_detach(pthread_t tid); //成功返回0，否则返回错误编码

在默认情况下，线程的终止状态会保存直到对该线程调用`pthread_join`。如果线程已经被分离，线程的底层存储资源可以在线程终止时立即被收回。先线程被分离后，我们不能用`pthread_join`函数等待它的终止状态，因为对分离状态的线程调用`pthread_join`会产生未定义行为。可以通过上述的`pthread_detach`分离线程

### 线程同步

线程需要同步的原因：

* 当一个线程修改变量时，其他线程在读取这个变量的时可能会看到一个不一致的值
* 当两个或多个线程试图在同一时间修改同一变量时，变量的值可能与预期不一样
* 程序使用变量的方式并非原子操作

**互斥量（mutex）**

互斥量本质上是一把锁，在访问共享资源前对互斥量进行设置，在访问完成后释放互斥量，确保同一时间只有一个线程访问数据。任何试图再次对互斥量在锁的线程都会被阻塞直到当前线程释放互斥量

互斥量用`pthread_mutex_t`数据类型表示，在使用互斥变量以前，必须首先对它进行初始化，可以把它设为常量`PTHREAD_MUTEX_INITIALIZER`（只适用于静态分配的互斥量），也可以通过调用`pthread_mutex_init`函数进行初始化。如果动态分配互斥量，在释放内存前需要调用`pthread_mutex_destroy`

	#include <pthread.h>
	int pthread_mutex_init(pthread_mutex_t *restrict mutex,
			const pthread_mutexattr_t *restrict attr);
	int pthread_mutex_destroy(pthread_mutex_t *mutex);
	//两个函数成功都返回0，否则返回错误号

要用默认的属性初始化互斥量，只需把`pthread_mutex_init`的`attr`参数设置为`NULL`

对互斥量加锁的方法：

	#include <pthread.h>
	int pthread_mutex_lock(pthread_mutex_t *mutex);
	int pthread_mutex_trylock(pthread_mutex_t *mutex);
	int pthread_mutex_unlock(pthread_mutex_t *mutex);
	//成功返回0，否则返回错误号

如果线程不希望被阻塞，可以通过`pthread_mutex_trylock`尝试对互斥量进行加锁

**避免死锁**



