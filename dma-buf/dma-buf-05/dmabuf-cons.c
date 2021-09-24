#include "stdio.h"
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <signal.h>
#include <sys/mman.h>

#include "task.h"
#define BUF_SIZE (0x1024)


void cons_dump(int pfd)
{
	char  *addr = mmap(NULL, BUF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, pfd, 0);
	// printf("%s", addr);
	munmap((void *)addr, BUF_SIZE);
}

int main(int argc, char *argv[])
{
	int sockfd, confd;
	// 1. 创建socket server
	int ret = init_socket_server(&sockfd, &confd);
	if (ret < 0) {
		printf("Server init failed\n");
		return ret;
	}
	// 2. 接收任务，并返回
	struct consumer_task ptask;
	printf("Start consumer thread...,size %lu\n", sizeof(ptask));
	while(1) {
		ret = recv(confd, &ptask, sizeof(ptask), 0);
		if (ret < 0) {
			printf("recv return %d\n", ret);
		}
		printf("recv return %d,read %#x\n", ret, ptask.seq);
		// ptask.seq = 0x5a5a5a5a;
		// cons_dump(ptask.seq);

		ret = send(confd, &ptask, sizeof(ptask), 0);
		printf("send return %d,send %#x\n", ret, ptask.seq);
	}
	printf("End producer thread...\n");

	sleep(10);
	return 0;
}


