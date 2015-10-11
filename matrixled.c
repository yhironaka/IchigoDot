/***************************************************
** matrixled.c
** Matrix LED Driver
** 
** IchigoDotS 
** License: CC BY yhironaka@gmail.com
** 
** This program was modified by Yasuhisa Hironaka.
** Original code was written by Taisuke Fukuno.
** WAREABLE MATRIX min
** License: CC BY http://fukuno.jig.jp/
***************************************************/

#include "main.h"
#include "matrixled.h"
#include "LPC1100.h"
#include "utils.h"

char	g_disp_buf[8];
char 	g_matcnt;
char 	g_matbuf[8];
short 	g_d0def = 0;
short 	g_d1def = 0;
short 	g_d0mask = 0;
short 	g_d1mask = 0;
short   g_rotate = ROTATE;

// -- LEDタイプ間違え対応 --
#if TYPE == 0
	const char g_xpos[] = { 4, 2, 109, 3, 100, 108, 5, 103 };
	const char g_ypos[] = { 6, 8, 9, 102, 7, 101, 11, 10 };
#endif

#if TYPE == 1
/*
	x	y
0	0_6	0_4
1	1_2	1_1
2	0_11	1_0
3	1_3	0_2
4	0_9	0_5
5	0_10	0_8
6	0_7	1_9
7	0_3	1_8
*/
	const char g_xpos[] = { 6, 102, 5, 103, 9, 10, 4, 3 };
	const char g_ypos[] = { 7, 101, 100, 2, 11, 8, 109, 108 };
#endif

void matrixled_init() {
	g_matcnt = 0;
	for (int i = 0; i < 8; i++) g_matbuf[i] = 0;
	
	IOCON_PIO0_7 		= 0x000000d0;	GPIO0DIR |= 1 <<  7;
	IOCON_PIO0_4 		= 0x000000d0;	GPIO0DIR |= 1 <<  4;
	IOCON_PIO0_3 		= 0x000000d0;	GPIO0DIR |= 1 <<  3;
	IOCON_PIO0_2 		= 0x000000d0;	GPIO0DIR |= 1 <<  2;
	IOCON_PIO1_9 		= 0x000000d0;	GPIO1DIR |= 1 <<  9;
	IOCON_PIO1_8 		= 0x000000d0;	GPIO1DIR |= 1 <<  8;
	IOCON_PIO0_8 		= 0x000000d0;	GPIO0DIR |= 1 <<  8;
	IOCON_PIO0_9 		= 0x000000d0;	GPIO0DIR |= 1 <<  9;
	IOCON_SWCLK_PIO0_10 = 0x000000d1;	GPIO0DIR |= 1 << 10;
	IOCON_R_PIO0_11 	= 0x000000d1;	GPIO0DIR |= 1 << 11;
	IOCON_PIO0_5 		= 0x000000d0;	GPIO0DIR |= 1 <<  5;
	IOCON_PIO0_6 		= 0x000000d0;	GPIO0DIR |= 1 <<  6;
	IOCON_R_PIO1_0 		= 0x000000d1;	GPIO1DIR |= 1 <<  0;
	IOCON_R_PIO1_1 		= 0x000000d1;	GPIO1DIR |= 1 <<  1;
	IOCON_R_PIO1_2 		= 0x000000d1;	GPIO1DIR |= 1 <<  2;
	IOCON_SWDIO_PIO1_3 	= 0x000000d1;	GPIO1DIR |= 1 <<  3;
	
	/*
	x		y
0	PIO0_4	PIO0_6
1	PIO0_2	PIO0_8
2	PIO1_9	PIO0_9
3	PIO0_3	PIO1_2
4	PIO1_0	PIO0_7
5	PIO1_8	PIO1_1
6	PIO0_5	PIO0_11
7	PIO1_3	PIO0_10
	*/
	// 	y->x // x = HIGH
	//            BA9876543210
	for (int i = 0; i < 8; i++) {
		int nx = g_xpos[i];
		if (nx < 100) {
			g_d0def  |= 1 << nx;
			g_d0mask |= 1 << nx;
		} else {
			g_d1def  |= 1 << (nx - 100);
			g_d1mask |= 1 << (nx - 100);
		}
		int ny = g_ypos[i];
		if (ny < 100) {
			g_d0mask |= 1 << ny;
		} else {
			g_d1mask |= 1 << (ny - 100);
		}
	}
	GPIO0MASKED[g_d0mask] = g_d0def;
	GPIO1MASKED[g_d1mask] = g_d1def;
}

void matrixled_on(int x, int y) {
	if (x < 0)	x = 0;
	if (y < 0)	y = 0;
	if (x > 7)	x = 7;
	if (y > 7)	y = 7;
	
	// rotate
	if (g_rotate == 0) {
		x = 7 - x;
	}
	else if (g_rotate == 2) {
		y = 7 - y;
	}
	else if (g_rotate == 1) {
		int t = x;
		x = y;
		y = t;
	}
	else if (g_rotate == 3) {
		int t = x;
		x = 7 - y;
		y = 7 - t;
	}

	// view	
	int d0 = g_d0def; // 0b000000111100;
	int d1 = g_d1def; // 0b001100001001;
	int ny = g_ypos[y];
	if (ny < 100) {
		d0 |= 1 << ny;
	} else {
		d1 |= 1 << (ny - 100);
	}
	int nx = g_xpos[x];
	if (nx < 100) {
		d0 &= ~(1 << nx);
	} else {
		d1 &= ~(1 << (nx - 100));
	}
	GPIO0MASKED[g_d0mask] = d0;
	GPIO1MASKED[g_d1mask] = d1;
}

void matrixled_off() {
	GPIO0MASKED[g_d0mask] = g_d0def;
	GPIO1MASKED[g_d1mask] = g_d1def;
}

void matrixled_tick() {
	int  j = g_matcnt / 8;
	int  i = g_matcnt % 8;
	char d = g_matbuf[j];
	if (d & (1 << i)) {
		matrixled_on(i, j);
	} else {
		matrixled_off();
	}
	g_matcnt++;
	if (g_matcnt == 64)	g_matcnt = 0;
}

void set_matrix(const char* data) {
	for (int j = 0; j < 8; j++) {
		g_matbuf[j] = data[j];
	}
}

void get_matrix(char* data) {
	for (int j = 0; j < 8; j++) {
		data[j] = g_matbuf[j];
	}
}

void PSET(int x, int y) {
	g_disp_buf[x & 7] |= 1 << (y & 7);
}

void PRESET(int x, int y) {
	g_disp_buf[x & 7] &= ~(1 << (y & 7));
}

void CLS(int n) {
	for (int i = 0; i < 8; i++) g_disp_buf[i] = n ? 0xff : 0;
}

void FILL(const char *hex_string) {
	decode_top2bottom(hex_string, g_disp_buf);
}

void FLUSH() {
	set_matrix(g_disp_buf);
}

void put_matrix(const char* s, int dx, int dy, int dw, int dh) {
	char src[8];
	decode_top2bottom(s, src);
	for (int i = 0; i < dw; i++) {
		for (int j = 0; j < dh; j++) {
			int x = dx + i;
			int y = dy + j;
			if (x < 0 || y < 0 || x > 7 || y > 7) continue;
			PRESET(x, y);
			int n = (src[i] >> j) & 1;
			if (n) PSET(x, y);
		}
	}
}
