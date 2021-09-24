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
#include "task.h"


#define IOC_MAGIC	'E'
#define OPEN_DMABUF_FD	_IO(IOC_MAGIC, 0)
#define CLEAR_EXCL_FENCE	_IO(IOC_MAGIC, 1)
#define SET_WM_FD  _IO(IOC_MAGIC, 2)


#define SET_DMABUF_FD	_IO(IOC_MAGIC, 10)
#define TEST_DMABUF_RW	_IO(IOC_MAGIC, 11)
#define UNSET_DMABUF_FD	_IO(IOC_MAGIC, 12)
#define TEST_DMABUF_USED	_IO(IOC_MAGIC, 13)

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
		close(ifd);
		ifd = 0;
	}

	printf("Close efd %d\n", efd);
	if (efd) {
		printf("Close efd\n");
		close(efd);
		efd = 0;
	}
	exit(0);
}

#define BUF_SIZE (0x1024)
void cons_dump(int pfd)
{
	char  *addr = mmap(NULL, BUF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, pfd, 0);
	printf("%s", addr);
	munmap((void *)addr, BUF_SIZE);
}

int main(int argc, char *argv[])
{
	int ret = -EINVAL;
	int i = 0;
	struct consumer_task *ptask;

	printf("version is %d\n", 0);
	efd = open("/dev/exporter", O_RDWR);
	ret = socket_client_thread();
	if (efd < 0 ||  ret < 0)
		printf("open exp/imp fd failed,  e is %d,  client ret is %d\n", efd, ret);
	else
		printf("open exp/imp fd success,  e is %d, client ret is %d\n", efd, ret);

#if 1
	for (i=0; i<DBUF_MAX_COUNT; ++i) {
		ret = ioctl(efd, OPEN_DMABUF_FD, &dfd[i]);
		if (ret < 0) {
			printf("get dma-buf fd failed\n");
			goto failed;
		} else {
			printf("%d get dma-buf fd success, fd %d\n", i, dfd[i]);
			fd_index[dfd[i]] = i; // 建立fd-->index得索引
		}
	}
	signal(SIGINT, dmabuf_sigint); // SIG_INT中关闭所有描述符

	while(1) {
		printf("Input enter to continue...\n");
		getchar();
		struct pollfd fds[DBUF_MAX_COUNT] = {0};
		int pfd = dfd[get_free_fd()]; // 获取空闲fd，并设置busy
		ret = ioctl(efd, SET_WM_FD, &pfd);
		for (i=0; i<1; ++i) {
			fds[i].fd = pfd;
			fds[i].events = POLLIN;
		}
		ret = poll(fds, 1, -1);
		if (ret > 0) { // 有事件发生
			for (i=0; i<1; ++i) {
				if (fds[i].revents  & (POLLERR | POLLNVAL)) {
					printf("dbuf %d recv error\n", i);
				}
				if (fds[i].revents & POLLIN) {
					printf("dbuf %d return,fd  is %d\n", i, fds[i].fd);
					cons_dump(fds[i].fd);

					ptask = create_task(fds[i].fd);
					if (!ptask) {
						fprintf(stderr,"Short of memory, wait and try\n");
						continue;
					}
					printf("main:start push %d\n", fds[i].fd);
					socket_push_task(ptask);
				}
			}
		} else if (ret == 0) { // 超时
			printf("poll timeout\n");
		} else { // 异常
			printf("poll abort!!!\n");
			goto failed;
		}
	}
#else
	while (1) {
		ptask = create_task(0xa5a5a5a5);
		if (!ptask) {
			fprintf(stderr,"Short of memory, wait and try\n");
			continue;
		}
		printf("main:start push %#x\n", ptask->seq);
		socket_push_task(ptask);

		sleep(3);
	}
	return 0;
#endif
	ret = 0;

failed:
	dmabuf_sigint(0);

	return ret;
}
