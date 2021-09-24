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

int efd;
int ifd;
int dfd;


static inline int sync_wait(int fd, int timeout, int flags)
{
	struct pollfd fds = {0};
	int ret;

	fds.fd = fd;
	fds.events = POLLIN;
	printf("Start poll\n");
	do {
		ret = poll(&fds, 1, timeout);
		if (ret > 0) {
			if (fds.revents & (POLLERR | POLLNVAL)) {
				errno = EINVAL;
				return -1;
			}
			printf("poll ok, event is %#x\n", fds.revents);
			return 0;
		} else if (ret == 0) {
			errno = ETIME;
			printf("poll timeout\n");
			return -1;
		}
	} while (ret == -1 && (errno == EINTR || errno == EAGAIN));

	return ret;
}

void dmabuf_sigint(int signo)
{
	printf("Recv sigint signal\n");
	printf("Close ifd %d\n", ifd);
	if (ifd) {
		printf("Close ifd\n");
		//ioctl(ifd, UNSET_DMABUF_FD, &dfd);
		close(ifd);
		ifd = 0;
	}
	sleep(1);

	printf("Close dfd %d\n", dfd);
	if (dfd) {
		printf("Close dfd\n");
		close(dfd);
		dfd = 0;
	}
	sleep(1);
	printf("Close efd %d\n", efd);
	if (efd) {
		printf("Close efd\n");
		close(efd);
		efd = 0;
	}
	sleep(1);

	exit(0);
}

int main(int argc, char *argv[])
{
	int ret = -EINVAL;

	efd = open("/dev/exporter", O_RDWR);
	ifd = open("/dev/importer", O_RDWR);
	if (dfd < 0 || ifd < 0)
		printf("open exp/imp fd failed,  e is %d, i is %d\n", efd, ifd);
	else
		printf("open exp/imp fd success,  e is %d, i is %d\n", efd, ifd);


	if (ioctl(efd, OPEN_DMABUF_FD, &dfd) < 0) {
		printf("get dma-buf fd failed\n");
		goto failed;
	} else
		printf("get dma-buf fd success, fd is %d\n", dfd);


	if (ioctl(ifd, SET_DMABUF_FD, &dfd) < 0) {
		printf("set dma-buf fd failed\n");
		goto failed;
	}

	signal(SIGINT, dmabuf_sigint); // SIG_INT中关闭所有描述符

#if 0
	// 测试
	if (ioctl(ifd, TEST_DMABUF_RW, 0) < 0) {
		printf("test dma-buf failed\n");
		goto failed;
	}
#endif

	while(1) {
		// ioctl(efd, CLEAR_EXCL_FENCE, 0);
		printf("Waiting out-fence to be signaled on USER side ...\n");
		ret = sync_wait(dfd, -1, POLLIN);  // 等待生产者产生完成
		if (ret != 0) {
			printf("dpu产生异常\n");
			break;
		}
		printf("dpu产生完成\n");
		ioctl(ifd, TEST_DMABUF_USED, 0);
		ret = sync_wait(dfd, -1, POLLOUT);  // 等待消费者使用完成
		if (ret != 0) {
			printf("vpu产生异常\n");
			break;
		}
		ioctl(efd, CLEAR_EXCL_FENCE, 0);
		printf("vpu使用完成\n");
	}

	if (ioctl(ifd, UNSET_DMABUF_FD, 0) < 0) {
		printf("unset dma-buf fd failed\n");
		goto failed;
	}
	ret = 0;

failed:
	dmabuf_sigint(0);

	return ret;
}
