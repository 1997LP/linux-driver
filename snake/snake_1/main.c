#include "pic_show.h"
#include "snake.h"
#include  <pthread.h>
#include  <signal.h>
#include  <sys/types.h>
pthread_t 	MOVE;      
pthread_t 	TOUCH;      
pthread_t 	MUSIC;
 
int main(int argc,char *argv[])
{
	show_window();
	pthread_create(&TOUCH, NULL,KEY_value,    NULL);
	pthread_create(&MOVE , NULL,play_game,    NULL);
	pthread_create(&MUSIC, NULL,music_pthread,NULL);
	pthread_join  ( MOVE , NULL);
	pthread_join  ( TOUCH, NULL);
	pthread_join  ( MUSIC, NULL);
	return 0;
}



