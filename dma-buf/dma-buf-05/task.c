/*************************************************************************
    > File Name: test01.c
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2021-08-27-21:57:40
    > Func:  多任务间的同步，为了方便移植，尽可能的少用私有库接口
 ************************************************************************/

#include "task.h"

static struct task_manager *g_pct;
static unsigned long fd_busy = 0;
unsigned int fd_index[MAXLINE];  // 记录fd得索引

static void *producer_func(void *arg)
{
	int ret = 0;
	int i = 0;
	struct task_manager *pct = arg;
	struct consumer_task ptask;

	printf("Start producer thread...\n");
	while(1) {
		ret = recv(pct->sockcfd, &ptask, sizeof(ptask), 0);
		if (ret < 0) {
			printf("recv return %d\n", ret);
		} else if (ret == 0) {
			printf("socket server break...\n");
			pthread_exit(0);
		}
		printf("!!!recv return %d,read %#x, index %d\n", ret, ptask.seq, fd_index[ptask.seq]);
		fd_busy &= ~(1<<fd_index[ptask.seq]);
	}
	printf("End producer thread...\n");
	pthread_exit(0);
}

static void consumer_func_exit(void *arg)
{
	struct consumer_thread *pct = arg;
	printf("Consumer Thread quit\n");
}

static void *consumer_func(void *arg)
{
	struct task_manager *pct = arg;
	struct consumer_task *ptask = NULL;
	struct consumer_task *ttask = NULL;

	pthread_cleanup_push(consumer_func_exit, arg);
	printf("Start consumer thread...\n");
	while(1) {
		pthread_mutex_lock(&pct->c_lock);
		while(!pct->task_cnt) {
			pthread_cond_wait(&pct->cond, &pct->c_lock); // 释放锁，等待，加锁
		}
		// pop task
		list_for_each_entry_safe(ptask, ttask, &pct->head, node) {
			list_del(&ptask->node);
			// consumer: dump data
			printf("POP: task pid %#x, task_cnt %ld\n", ptask->seq, pct->task_cnt);
			send(pct->sockcfd, ptask, sizeof(*ptask), 0);
			free(ptask);
			--pct->task_cnt;
		}
		pthread_mutex_unlock(&pct->c_lock);
	}
	printf("End consumer thread...\n");
	pthread_cleanup_pop(0);
	pthread_exit(0);
}

static int init_task_thread(struct task_manager *pct,
		void *(*cfn) (void *), void *(*pfn) (void *), void *arg)
{
	if (!pct)
		return -EINVAL;
	INIT_LIST_HEAD(&pct->head);

	pthread_attr_init(&pct->attr);
	pthread_cond_init(&pct->cond, NULL);
	pthread_mutex_init(&pct->c_lock, NULL);
	pthread_mutex_init(&pct->p_lock, NULL);

	// Set DETACH, Success return 0
	pthread_attr_setdetachstate(&pct->attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&pct->ctid, &pct->attr, cfn, arg);
	pthread_create(&pct->ptid, &pct->attr, pfn, arg);
	pthread_attr_destroy(&pct->attr);
}

int init_socket_client(void)
{
	int ret;
	struct sockaddr_in servaddr;
	int socket_cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_cfd < 0) {
		printf("client socket open error\n");
		return socket_cfd;
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	// servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	servaddr.sin_port = htons(DEFAULT_PORT);//设置的端口为DEFAULT_PORT
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

	if(connect(socket_cfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
		printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
		return -1;
	}
	return socket_cfd;
}

int init_socket_server(int *sockfd, int *confd)
{
	int ret;
	struct sockaddr_in servaddr;
	struct sockaddr_in clientaddr, localaddr;
	unsigned int sockaddr_len;
	int	socket_sfd, connect_fd;
	int reuse = 1;

	socket_sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_sfd < 0) {
		printf("server socket open error\n");
		return socket_sfd;
	}
	setsockopt(socket_sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	*sockfd = socket_sfd;

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	servaddr.sin_port = htons(DEFAULT_PORT);//设置的端口为DEFAULT_PORT

	//将本地地址绑定到所创建的套接字上
	if(bind(socket_sfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		printf("bind socket error: %s(errno: %d)\n", strerror(errno),errno);
		goto err_bind;
	}

	//开始监听是否有客户端连接
	if( listen(socket_sfd, 10) == -1){
		printf("listen socket error: %s(errno: %d)\n", strerror(errno),errno);
		goto err_listen;
	}
	printf("======waiting for client's request======\n");

	while(1){
		//阻塞直到有客户端连接，不然多浪费CPU资源。
		if( (connect_fd = accept(socket_sfd, (struct sockaddr*)NULL, NULL)) == -1){
			printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
			continue;
		}

		sockaddr_len = sizeof(localaddr);
		if (getsockname(connect_fd, (struct sockaddr *)&localaddr, &sockaddr_len) == 0){
			printf("connected server address = %s:%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
		}

		sockaddr_len = sizeof(localaddr);
		if (getpeername(connect_fd, (struct sockaddr *)&clientaddr, &sockaddr_len) == 0){
			printf("connected peer address = %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		}

		// 创建子线程用来处理 任务
		// 我们目前测试用一个任务，所以先不创建多线程 了
		*confd = connect_fd;
		return 0;
	}

err_listen:
err_bind:
	close(socket_sfd);
}

static void exit_consumer_thread(struct task_manager *pct)
{
	if (!pct)
		return;
	pthread_cond_destroy(&pct->cond);
	pthread_mutex_destroy(&pct->c_lock);
	pthread_mutex_destroy(&pct->p_lock);
}

struct consumer_task *create_task(unsigned int seq)
{
	struct consumer_task *ptask = calloc(1, sizeof(struct consumer_task));
	if (!ptask)
		return NULL;
	INIT_LIST_HEAD(&ptask->node);
	ptask->seq = seq;
	return ptask;
}
static void push_task(struct task_manager *pct, struct consumer_task *ptask)
{
	if (!ptask)
		return;
	pthread_mutex_lock(&pct->c_lock);
	list_add_tail(&ptask->node, &pct->head);
	++pct->task_cnt;
	printf("PUSH: task send %#x, task_cnt %ld\n", ptask->seq, pct->task_cnt);
	pthread_mutex_unlock(&pct->c_lock);
	pthread_cond_signal(&pct->cond);
}

void test_eixt(void)
{
	printf("进程退出\n");
	printf("进程退出1-线程取消\n");
	pthread_cancel(g_pct->ctid);
	pthread_cancel(g_pct->ptid);
	printf("进程退出2-资源释放\n");
	exit_consumer_thread(g_pct);
	printf("进程退出3-释放结构\n");
	free(g_pct);
}

void socket_push_task(struct consumer_task *ptask)
{
	if (!ptask)
		return;
	push_task(g_pct, ptask);
}

int socket_client_thread(void)
{
	int ret = 0;
	ret = init_socket_client();
	if (ret < 0) {
		printf("con't link socket server\n");
		return ret;
	}
	g_pct = calloc(1, sizeof(*g_pct));
	g_pct->sockcfd = ret;
	srandom((unsigned int)time(NULL));
	init_task_thread(g_pct, consumer_func, producer_func, g_pct);
	atexit(test_eixt);
}

static inline int ffs_8(unsigned char word)
{
	int num = 0;
	if (word & 0x0f == 0) {
		num += 4;
		word >>= 4;
	}
	if (word & 0x03 == 0) {
		num += 2;
		word >> 2;
	}
	if (word & 0x1 == 0) {
		num += 1;
	}
	return num;
}
static inline int ffz_8(unsigned char val)
{
	return ffs_8(~val);
}

int get_free_fd()
{
	while(fd_busy & 0xff) {
		usleep(16700);
	}
	return ffz_8(fd_busy);
}
