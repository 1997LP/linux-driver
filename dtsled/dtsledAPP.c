#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int ret=0;
    int fd;
    unsigned char databuf[1];
    if(argc<3){
        printf("error pparam\r\n");
        return -1;
    }
    fd=open(argv[1],O_RDWR);
    if(fd<0){
        printf("open %s error ",argv[1]);
    }
    else {
        databuf[0]=atoi(argv[2]);
        ret=write(fd,databuf,sizeof(databuf));
        if(ret<0){
            printf("write data error\r\n");
            return -2;
        }
    }
    close(fd);
    return 0 ;
}