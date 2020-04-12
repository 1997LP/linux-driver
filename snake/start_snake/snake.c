
#include  "snake.h"
#include  <unistd.h>
#include  <pthread.h>
#include  <sys/wait.h>

void show_window(void)
{
    int x,y;
    pid_t pid;
    while(1)
    {
        lcd_init();
        bmp_show(BCK_PIC,0,0);
        get_xy(&x,&y);
        if( x>400 && y>50 )
        {               //普通模式
            lcd_free();
            pid=fork();
            if(pid==0)execl("./a.out",  "./a.out", NULL);  
            if(pid<0)
            {
                printf("  普通模式创建失败!!\r\n");
                _exit(0);
            }
        }
        else if(x<400 && y>50) 
        {               //挑战模式  
            lcd_free();
            pid=fork();
            if(pid==0)execl("./b.out",  "./b.out", NULL);  
            if(pid<0)
            {
                printf("  挑战模式创建失败!!\r\n");
                _exit(0);
            } 
        }  
        wait(NULL);
        printf("游戏结束！！！\r\n");
    }
    
}
