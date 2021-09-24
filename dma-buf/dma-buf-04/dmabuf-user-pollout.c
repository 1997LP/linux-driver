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

#define IOC_MAGIC	'E'
#define OPEN_DMABUF_FD	_IO(IOC_MAGIC, 0)
#define CLEAR_EXCL_FENCE	_IO(IOC_MAGIC, 1)

#define SET_DMABUF_FD	_IO(IOC_MAGIC, 10)
#define TEST_DMABUF_RW	_IO(IOC_MAGIC, 11)
#define UNSET_DMABUF_FD	_IO(IOC_MAGIC, 12)
#define TEST_DMABUF_USED	_IO(IOC_MAGIC, 13)

#define DBUF_MAX_COUNT (3)

int efd;
int ifd;
int dfd[DBUF_MAX_COUNT];

void dmabuf_sigint(int signo)
{
	int i = 0;
	printf("Recv sigint signal\n");


	printf("Close ifd %d\n", ifd);
	if (ifd) {
		printf("Close ifd\n");
		// ioctl(ifd, UNSET_DMABUF_FD, 0);
		close(ifd);
		ifd = 0;
	}

	printf("Close efd %d\n", efd);
	if (efd) {
		printf("Close efd\n");
		close(efd);
		efd = 0;
	}
	sleep(1);
	printf("Close ifd %d\n", ifd);
	exit(0);
}

int main(int argc, char *argv[])
{
	int ret = -EINVAL;
	int i = 0;

	printf("version is %d\n", 0);
	efd = open("/dev/exporter", O_RDWR);
	ifd = open("/dev/importer", O_RDWR);
	if (dfd < 0 || ifd < 0)
		printf("open exp/imp fd failed,  e is %d, i is %d\n", efd, ifd);
	else
		printf("open exp/imp fd success,  e is %d, i is %d\n", efd, ifd);


	if (ioctl(efd, OPEN_DMABUF_FD, dfd) < 0) {
		printf("get dma-buf fd failed\n");
		goto failed;
	} else
		printf("get dma-buf fd success, fd %d-%d-%d\n", dfd[0], dfd[1], dfd[2]);

	if (ioctl(ifd, SET_DMABUF_FD, dfd) < 0) {
		printf("set dma-buf fd failed\n");
		goto failed;
	}
	signal(SIGINT, dmabuf_sigint); // SIG_INT中关闭所有描述符

#if 1
	// 刚开始只监听POLLIN
	short dfd_st[DBUF_MAX_COUNT] ={0};
	for (i=0; i<DBUF_MAX_COUNT; ++i) {
		dfd_st[i] = POLLOUT;
	}

	while(1) {
		struct pollfd fds[DBUF_MAX_COUNT] = {0};
		for (i=0; i<DBUF_MAX_COUNT; ++i) {
			fds[i].fd = dfd[i];
			fds[i].events = dfd_st[i];
		}

		ret = poll(fds, 3, -1);
		if (ret > 0) { // 有事件发生
			for (i=0; i<DBUF_MAX_COUNT; ++i) {
				if (fds[i].revents  & (POLLERR | POLLNVAL)) {
					printf("dbuf %d recv error\n", i);
				}
				if (fds[i].revents & POLLOUT) {
					printf("dbuf %d return\n", i);
					ioctl(efd, CLEAR_EXCL_FENCE, &i);  // 假设没有消费者哈
				}
			}
		} else if (ret == 0) { // 超时
			printf("poll timeout\n");
		} else { // 异常
			printf("poll abort!!!\n");
			goto failed;
		}
	}
#endif
	ret = 0;

failed:
	dmabuf_sigint(0);

	return ret;
}
