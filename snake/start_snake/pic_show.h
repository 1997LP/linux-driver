#ifndef 		__PIC_SHOW_H
#define 		__PIC_SHOW_H
/

#include          <stdio.h>
#include          <stdlib.h>
#include          <string.h>
#include          <math.h>
#include          <time.h>
#include          <unistd.h>
#include          <sys/types.h>
#include          <sys/stat.h>
#include          <fcntl.h>
#include 	  <linux/input.h> //触摸屏

#define		GAMEOVER	"./SNK_pic/over.bmp" 
#define		H_PIC		"./SNK_pic/h.bmp"
#define		S_PIC		"./SNK_pic/6.bmp"
#define		BCK_PIC		"./SNK_pic/start.bmp"

int 	bmp_show(char * pic_path, int x0, int y0);
void 	lcd_init(void);
void 	lcd_free(void);
int 	bmp_show_bck(int x0, int y0, int W, int H);
void  	get_xy( int * x, int *y);

#endif
