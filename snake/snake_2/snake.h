#ifndef 		__SNAKE_INFO_H
#define 		__SNAKE_INFO_H

#include          <stdio.h>
#include          <stdlib.h>
#include          <stdint.h>
#include          <string.h>
#include          <strings.h>
#include          <math.h>
#include          <sys/mman.h>
#include          <time.h>
#include          <unistd.h>
#include          <sys/types.h>
#include          <sys/stat.h>
#include          <fcntl.h>
#include          <pthread.h>
#include          <sys/stat.h>
#include          <semaphore.h>
#include          <signal.h>

#include 	      <sys/time.h>
#include 	      "pic_show.h"

#define u8        unsigned char
#define u16       unsigned short
#define u32       unsigned int

#define s8        char
#define s16       short
#define s32       signed 

#define turn_LEFT  0x01
#define turn_right 0x02
#define turn_up    0x03
#define turn_down  0x04
#define MAX_NUM    66

#define success    0x05
#define fail       0x06

typedef struct {
    s16 x_point;
    s16 y_point;
    u8 stat;
}snake_P;

extern snake_P SNAK[MAX_NUM]; 
extern u16 snake_front;
extern u16 snake_rear;
extern u16 SRAND;
extern int X0,Y0;

extern sem_t       touch_condition;
extern u8 OVER;
void body_point(u16 x0,u16 y0, u16 status,u16 color);
void show_window(void);
void snake_init(void);
void R_and(void);
void *play_game(void * p);
void *KEY_value(void *p);
void *music_pthread(void *p);
#endif

