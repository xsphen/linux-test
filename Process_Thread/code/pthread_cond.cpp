#include "apue.h"
#include <pthread.h>

/* 初始化条件变量和互斥量 */
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* 一个唤醒条件 */
bool awake = false;

/* 新创建线程执行的入口 */
void *func(void *arg)
{
	/* 首先锁住互斥量，再等待条件变量 */
	pthread_mutex_lock(&mutex);
	pthread_t tid = pthread_self();
	printf("%lu got the mutex...\n", (unsigned long)tid);
	while (!awake) {
		pthread_cond_wait(&condition, &mutex);
		printf("i am %lu, i am wating...\n", (unsigned long)tid);
	}

	/* 在等待条件变量的时候，先把对互斥量解锁，自己挂起 */
	pthread_mutex_unlock(&mutex);
}
int main()
{
	/* 创建新线程 */
	pthread_t tid;
	pthread_create(&tid, NULL, func, NULL);

	/* 睡眠，保证新线程先对互斥量加锁 */
	sleep(2);

	/* 主线程在新建线程等待条件时获得锁 */
	pthread_mutex_lock(&mutex);
	awake = true;
	printf("i am main.\n");
	pthread_mutex_unlock(&mutex);

	/* 条件完成，唤醒在等待的线程 */
	pthread_cond_signal(&condition);

	/* 睡眠，让醒过来的线程先执行 */
	sleep(2);
	exit(0);
}
