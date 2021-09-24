#ifndef __TASK_H_
#define __TASK_H_
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <sys/param.h>

#include <linux/magic.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <pthread.h>

#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>

#include "s_list.h"

struct task_manager {
	pthread_attr_t attr;

	pthread_t ctid;
	pthread_mutex_t c_lock;
	pthread_t ptid;
	pthread_mutex_t p_lock;

	pthread_cond_t  cond;
	pthread_condattr_t cattr;

	struct list_head head;
	unsigned long task_cnt;


	int sockcfd;
};

struct consumer_task {
	unsigned int seq;
	struct list_head node;
};

#define DEFAULT_PORT 8000
#define MAXLINE 4096
#define DBUF_MAX_COUNT (8)

extern void socket_push_task(struct consumer_task *ptask);
extern int socket_client_thread(void);
extern struct consumer_task *create_task(unsigned int seq);
extern unsigned int fd_index[MAXLINE];
extern int get_free_fd();
extern int init_socket_server(int *sockfd, int *confd);
#endif
