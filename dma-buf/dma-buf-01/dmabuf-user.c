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


#define IOC_MAGIC	'E'
#define OPEN_DMABUF_FD	_IO(IOC_MAGIC, 0)

#define SET_DMABUF_FD	_IO(IOC_MAGIC, 10)
#define TEST_DMABUF_RW	_IO(IOC_MAGIC, 11)
#define UNSET_DMABUF_FD	_IO(IOC_MAGIC, 12)


int main(int argc, char *argv[])
{
	int efd;
	int ifd;
	int dfd;
	int ret = -EINVAL;

	efd = open("/dev/exporter", O_RDWR);
	ifd = open("/dev/importer", O_RDWR);
	if (ifd < 0 || efd < 0) {
		printf("open exp/imp fd failed,  e is %d, i is %d\n", efd, ifd);
		return -1;
	} else {
		printf("open exp/imp fd success,  e is %d, i is %d\n", efd, ifd);
	}

	if (ioctl(efd, OPEN_DMABUF_FD, &dfd) < 0) {
		printf("get dma-buf fd failed\n");
		goto failed_get_dfd;
	} else
		printf("get dma-buf fd success, fd is %d\n", dfd);

	if (ioctl(ifd, SET_DMABUF_FD, &dfd) < 0) {
		printf("set dma-buf fd failed\n");
		goto failed;
	}

	if (ioctl(ifd, TEST_DMABUF_RW, 0) < 0) {
		printf("test dma-buf failed\n");
		goto failed;
	}

	if (ioctl(ifd, UNSET_DMABUF_FD, 0) < 0) {
		printf("unset dma-buf fd failed\n");
		goto failed;
	}
	ret = 0;
	getchar();

failed:
	close(dfd);
failed_get_dfd:
	if (efd > 0)
		close(efd);
	if (ifd > 0)
		close(ifd);

	return ret;
}
