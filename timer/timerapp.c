#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "linux/ioctl.h"

#define LED_OPEN     1          
#define LED_CLOSE    0            
#define CLOSE_CMD   (_IO(0xFE,0x1))
#define OPEN_CMD    (_IO(0xFE,0x2))
#define SET_PERIOD  (_IO(0xFE,0x3))

int main(int argc, char *argv[])
{
    int ret=0;
    int fd;
    unsigned int cmd=0;
    unsigned int timer=1000;
    if(argc<2){
        printf("error \r\n");
        return -1;
    }
    fd=open(argv[1],O_RDWR);
    if(fd<0){
        printf("open %s error ",argv[1]);
    }
    else {
        while(1){
            printf("请选择 (1)打开led,（2）关闭led，（3）修改定时时间\r\n");
            scanf("%d",&ret);
            switch(ret){
                case 1: cmd = OPEN_CMD;
                        break;
                case 2: cmd = CLOSE_CMD;
                        break;
                case 3: cmd = SET_PERIOD;
                        printf("请输入定时时间\r\n");
                        scanf("%d" ,&timer);
                        break;
                default: break;
            }
           ioctl(fd, cmd, timer);
        }
    }
    close(fd);
    return 0 ;
}
    



