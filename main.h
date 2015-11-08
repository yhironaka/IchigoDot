/*
main.h 
*/
#ifndef _MAIN
#define _MAIN

#include "utils.h"

#define VERSION_NUM "0.8.7" 

#define TYPE 0 // 通常
//#define TYPE 1 // マトリクスLEDの位置、間違えた時

#ifndef ROTATE
#define ROTATE 2
#endif
//#define ROTATE 1 // 右回転
//#define ROTATE 2 // 上下反転
//#define ROTATE 3 // 左回転

#define AUTO_SLEEP 1
#define PSG_ON 1

boolean ux_btn();
void 	deepPowerDown();
boolean ux_state();

int 	get_systick();
void 	set_systick(int systick);
void 	sleep_tick();
void 	no_sleep();

void 	wait(int n);
void 	WAIT(int n);

#endif
