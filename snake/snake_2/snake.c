
#include  "snake.h"
#include  <unistd.h>
#include  <pthread.h>

sem_t       touch_condition;
sem_t       GAME_OVER__;
u8  OVER=0;
u16 length_body=20;
u16 score=0;
u16 speed=10;
u16 SRAND=0;
snake_P SNAK[MAX_NUM]; 
u16     snake_front=0;
u16     snake_front_next;
u16     snake_rear=0;
snake_P target;
u8      KEY_state=turn_down;
u8      end_condition=0;
u8      eat_value=0;

extern pthread_t 	MOVE;      
extern pthread_t 	TOUCH;      
extern pthread_t 	MUSIC;
char *picture[26]={
                   "./SNK_pic/0.bmp" ,       "./SNK_pic/1.bmp" ,
                   "./SNK_pic/2.bmp" ,       "./SNK_pic/3.bmp" ,
                   "./SNK_pic/4.bmp" ,       "./SNK_pic/5.bmp" ,
                   "./SNK_pic/60.bmp",       "./SNK_pic/7.bmp" ,
                   "./SNK_pic/8.bmp" ,       "./SNK_pic/9.bmp" ,
                   "./SNK_pic/10.bmp",       "./SNK_pic/11.bmp",
                   "./SNK_pic/12.bmp",       "./SNK_pic/13.bmp",
                   "./SNK_pic/14.bmp",       "./SNK_pic/15.bmp",
                   "./SNK_pic/16.bmp",       "./SNK_pic/17.bmp",
                   "./SNK_pic/18.bmp",       "./SNK_pic/19.bmp",
                   "./SNK_pic/20.bmp",       "./SNK_pic/21.bmp",
                   "./SNK_pic/22.bmp",       "./SNK_pic/23.bmp",
                   "./SNK_pic/24.bmp",       "./SNK_pic/25.bmp",
                   };


const u16 x_rand[25]={0,20,40,60,80,100,120,140,160,
             180,200,220,240,260,280,300,320,340,360,380,
            400,420,440,460,480};
const u16 y_rand[24]={0,20,40,60,80,100,120,140,160,
             180,200,220,240,260,280,300,320,340,360,
             380,400,420,440,460};
                       
void R_and(void)
{
    int value=0;
    int con=1;
    while(1)
    {
        SRAND=rand()%25;
        if(SRAND>24) SRAND=24;
        target.x_point=x_rand[SRAND];
        if(SRAND>23)SRAND=23;
        target.y_point=y_rand[23-SRAND];
        for(value=0;value<MAX_NUM;value++)
        {
                if(SNAK[value].stat)
                {
                    if( (SNAK[value].x_point==target.x_point) && 
                    ( (SNAK[value].y_point==target.y_point) ) )
                    {
                        con=1;
                        break;
                    }
                }
        }
        if(  (target.x_point==140 && target.y_point<280) ||  (target.x_point==320 && target.y_point<180) ||
                     (target.x_point>=300 && target.y_point==280) ||
                     (target.x_point<200  && target.y_point==360) )
           {con=1;}

        if(!con)break;
        else con=0;
    }
	bmp_show(S_PIC,target.x_point,target.y_point);
}

void snake_init(void)
{
    u16 i=0;
    for(i=0;i<MAX_NUM;i++)
        SNAK[i].stat=0;
    for(i=0;i<5;i++)
    {
        SNAK[snake_front].y_point=20+length_body*i;
        SNAK[snake_front].x_point=60;
        SNAK[snake_front].stat=1;
		bmp_show(S_PIC,SNAK[snake_front].x_point, SNAK[snake_front].y_point);
        snake_front++;
    }
    snake_front--;
}

void show_window(void)
{
    int x,y;
    srand(time(NULL));
	lcd_init();
    printf("请按开始键\r\n");
    bmp_show(BCK_PIC,0,0);
    bmp_show("./SNK_pic/0.bmp", 550,110  );
    while(1)
    {
        get_xy(&x,&y);
        if( ((555<x)&&(x<750)) &&( (0<y) && (y<100)) )
        {
            printf("start_computer\r\n");
            break;
        }
    }
    snake_init();
    sem_init(&touch_condition,0,1);
    sem_init(&GAME_OVER__,0,1);
}


  int X0=0,Y0=0;
 void *KEY_value(void *p)
 {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
     while(1)
     {
        get_xy(&X0,&Y0);
        sem_wait(&touch_condition);
        if     ( X0< 600 && Y0>300)                                    KEY_state =turn_LEFT;
        else if( ((X0> 611 && X0<670) && ( Y0>380 && Y0<440)) )        KEY_state =turn_down ;
        else if( ((X0> 680 && X0<734) && ( Y0>380 && Y0<440)) )        KEY_state =turn_right;
        else if( ((X0> 610 && X0<670) && ( Y0>300 && Y0<360)) )        KEY_state =turn_up   ;
        else   {KEY_state=KEY_state;}
        sem_post(&touch_condition);
        
        sem_wait(&GAME_OVER__);
        if(OVER)
        {
            printf("get_OVER!!!!!!\r\n");
            return NULL;
        }
        sem_post(&GAME_OVER__);
     }
 }

 void *play_game(void *p)
 {
    int value=0;
    
    printf("申请资源\r\n");
    while(1)
    {
        
        snake_front_next=(snake_front+1)%MAX_NUM;
        if(snake_front_next==snake_rear)
        {
            end_condition=success;
            break;
            printf("success\r\n");
        }
        if(!eat_value)
        {
            R_and();//随机产生目标物
            eat_value=1;
        }
        sem_wait(&touch_condition);
        switch(KEY_state)
        {
            case turn_LEFT :
                            SNAK[snake_front_next].x_point=SNAK[snake_front].x_point-20;
                            SNAK[snake_front_next].y_point=SNAK[snake_front].y_point;
                            break;
            case turn_right:
                            SNAK[snake_front_next].x_point=SNAK[snake_front].x_point+20;
                            SNAK[snake_front_next].y_point=SNAK[snake_front].y_point;
                            break;
            case turn_up   :
                            SNAK[snake_front_next].x_point=SNAK[snake_front].x_point;
                            SNAK[snake_front_next].y_point=SNAK[snake_front].y_point-20;
                            break;
            case turn_down :
                            SNAK[snake_front_next].x_point=SNAK[snake_front].x_point;
                            SNAK[snake_front_next].y_point=SNAK[snake_front].y_point+20;
                            break;
            
            default:        break;
        }

        sem_post(&touch_condition);
        if( (SNAK[snake_front_next].x_point>=520) || (SNAK[snake_front_next].x_point<0) || 
                           (SNAK[snake_front_next].y_point<0) || (SNAK[snake_front_next].y_point>=480))
        {
                end_condition=fail;
                break;
        }
        if(  (SNAK[snake_front_next].x_point==140 && SNAK[snake_front_next].y_point<280) || 
                     (SNAK[snake_front_next].x_point==320 && SNAK[snake_front_next].y_point<180) ||
                     (SNAK[snake_front_next].x_point>=300 && SNAK[snake_front_next].y_point==280) ||
                     (SNAK[snake_front_next].x_point<200  && SNAK[snake_front_next].y_point==360) )
        {
                end_condition=fail;
                break;
         }


        for(value=0;value<MAX_NUM;value++)
        {
            if(SNAK[value].stat)
            {
                if( (SNAK[snake_front_next].x_point==SNAK[value].x_point) && 
                ( (SNAK[snake_front_next].y_point==SNAK[value].y_point) ) )
                {
                    end_condition=fail;
                    break;
                }
            }
        }
        
        if((SNAK[snake_front_next].x_point==target.x_point) && 
            ( (SNAK[snake_front_next].y_point==target.y_point) ))
        {
             snake_front=snake_front_next;
             SNAK[snake_front].stat=1;
             score++  ;
             if(score>=25)
             {
                 score=25;
                 bmp_show(SUCCESS,150 ,150 );
                 end_condition=success;
                 break;
             }
             bmp_show(picture[score], 550,110  );
             speed+=50  ;
             eat_value=0;
             printf("吃到了食物\r\n");       
        }
        else 
        {
             printf("处于查找状态\r\n");
             snake_front=snake_front_next;
             SNAK[snake_front].stat=1;
			 bmp_show(S_PIC,SNAK[snake_front].x_point,SNAK[snake_front].y_point);
			 bmp_show_bck(SNAK[snake_rear].x_point, SNAK[snake_rear].y_point,20,20);
             SNAK[snake_rear].stat=0;
             snake_rear=(snake_rear+1) %MAX_NUM;
        }
            usleep(60000-speed);
            usleep(60000-speed); 
            usleep(60000-speed); 
            usleep(60000-speed); 
            usleep(30000-speed); 
        if(end_condition==fail)break;
    }

    sem_wait(&GAME_OVER__);
    OVER=1;
    sem_post(&GAME_OVER__);
    switch(end_condition)
    {
        case fail:
                bmp_show(GAMEOVER,150,150);
                printf("failure!!!!!\r\n");
                pthread_cancel(TOUCH);
                pthread_cancel(	MUSIC);
                pthread_join(TOUCH,NULL);
                pthread_join(MUSIC,NULL);
                lcd_free();
                _exit(1);
                break;
        case success:
                printf("success!!!!!\r\n");
                pthread_cancel(TOUCH);
                pthread_cancel(	MUSIC);
                pthread_join(TOUCH,NULL);
                pthread_join(MUSIC,NULL);
                lcd_free();
                _exit(0);
                break;
        default:
                break;
    }
    return NULL;
}
 
void * music_pthread(void *p)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    system("madplay chengdu.mp3 ");
    while(1)
    {
        sem_wait(&GAME_OVER__);
        if(OVER) pthread_exit(NULL);
        sem_post(&GAME_OVER__);
    }

}
