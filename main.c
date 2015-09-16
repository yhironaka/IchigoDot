/***************************************************
** WAREABLE MATRIX min
** License: CC BY yhironaka@gmail.com
** 
** This program was modified by Yasuhisa Hironaka.
** Original code was written by Taisuke Fukuno.
** License: CC BY http://fukuno.jig.jp/
***************************************************/

#include <string.h>
#include "LPC1100.h"
#include "uart.h"
#include "xprintf.h"
#include "rnd.h"
#include "psg.h"
#include "iap.h"
#include "anim_ichigodot.h"

#define TYPE 0 // 通常
//#define TYPE 1 // マトリクスLEDの位置、間違えた時

#define AUTO_SLEEP 1
#define PSG_ON 1

#define ROTATE 2
//#define ROTATE 1 // 右回転
//#define ROTATE 2 // 上下反転
//#define ROTATE 3 // 左回転

#define ENEBLE_WDT 0 
/* 
0:12MHz .. 115200bpsでシリアル通信不可 57600bpsならok
1:2.3MHz 消費電流最小（これより遅いとちらつく、シリアル通信不可）

LPC1100L
	50MHz：7mA
	12MHz：2mA
	1MHz：840uA
	2.3MHz: 1.93mA ・・・WDTを使った最大クロック 1MHzからリニアと仮定、ほとんど12MHzと変わらない g_systick止めればもうちょっといい？
	9.3kHz: どうなる？？ WDTを使った最小クロック
*/

//#define SYSTICK_WAIT (!ENEBLE_WDT)
#define SYSTICK_WAIT 1

// util
#define boolean unsigned char
#define true 1
#define false 0
#define LEN_DATA 1024
#define N_FRAME ((LEN_DATA - 8) / 10)
#define WAIT(n) wait(n * 10)

// ----- struct -----
struct Frame {
	char frame[N_FRAME][8];
	short waitms[N_FRAME];
};

// ----- global variables -----
int 	g_uxbtn = 0;
int 	g_bkuxbtn = 0;
int 	g_timenobtn = 0;
short 	g_d0def = 0;
short 	g_d1def = 0;
short 	g_d0mask = 0;
short 	g_d1mask = 0;
char 	g_matbuf[8];
char 	g_matcnt;
volatile int 	g_systick;
char 	g_data[LEN_DATA];
struct Frame* 	g_fr;

boolean startsWith(char* s, char* key) {
	for (int i = 0;; i++) {
		char c1 = s[i];
		char c2 = key[i];
		if (c2 == '\0')
			return true;
		if (c1 == '\0')
			return false;
		if (c1 != c2)
			return false;
	}
}

int parseInt(char* s) {
	int res = 0;
	for (int i = 0;; i++) {
		char c = *s;
		if (c >= '0' && c <= '9') {
			res = res * 10 + (c - '0');
		} else {
			return res;
		}
		s++;
	}
}

int indexOf(char* s, char c) {
	for (int i = 0;; i++) {
		if (*s == 0)
			return -1;
		if (*s == c)
			return i;
		s++;
	}
}

void println(char* s) {
	for (;;) {
		char c = *s++;
		if (c == 0)
			break;
		uart0_putc(c);
	}
	uart0_putc('\n');
}

boolean ux_state() {
	return GPIO1MASKED[1 << 4] == 0;
}

void ux_tick() {
	int btn = ux_state();
	if (btn && !g_bkuxbtn) {
		g_uxbtn = 1;
	}
	g_bkuxbtn = btn;
}

boolean ux_btn() {
	//	return GPIO1MASKED[1 << 4] == 0;
	if (g_uxbtn) {
		g_uxbtn = 0;
		return 1;
	}
	return 0;
}

void ux_init() {
	IOCON_PIO1_4 = 0x000000d0;
	GPIO1DIR &= ~(1 << 4);
	IOCON_PIO1_5 = 0x000000d0;
	GPIO1DIR |= 1 << 5;
	
	psg_init();
}
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
#if ROTATE == 0
	x = 7 - x;
#elif ROTATE == 2
	y = 7 - y;
#elif ROTATE == 1
	int t = x;
	x = y;
	y = t;
#elif ROTATE == 3
	int t = x;
	x = 7 - y;
	y = 7 - t;
#endif

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

// ----- uart -----
void initUART() {
	uart0_init();
	xdev_out(uart0_putc);
	xdev_in(uart0_getc);
}

void sleep_tick();

// ----- systick -----
void InitSysTick(int hz) {
	SYST_RVR = SYSCLK / hz - 1;
	SYST_CSR = 0x07;
}
void SysTick_Handler(void) {
	g_systick++;
	matrixled_tick();
	ux_tick();
	
#if PSG_ON == 1	
	psg_tick();
#endif
#if AUTO_SLEEP == 1
	sleep_tick();
#endif
}
void wait(int n) {
	int endt = g_systick + n;
	for (;;) {
		if (g_systick > endt) break;
	}
}

// ----- util -----
void setMatrix(char* data) {
	for (int j = 0; j < 8; j++) {
		char d = data[j];
		for (int i = 0; i < 8; i++) {
			if (d & (1 << i)) {
				matrixled_on(i, j);
			} else {
				matrixled_off();
			}
#if SYSTICK_WAIT == 1
			wait(1);
#endif
		}
	}
	matrixled_off();
}

void setMatrix2(char* data) {
	for (int j = 0; j < 8; j++) {
		g_matbuf[j] = data[j];
	}
}

// 表示されている列をずらして変換
int decode(char* src, char* dst) {
	int len = 0;
	for (int i = 0; i < 16; i++) {
		int c = *(src + i);
		if 		(c >= '0' && c <= '9')	c -= '0';
		else if (c >= 'a' && c <= 'f')	c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')	c -= 'A' - 10;
		else break;
		len++;
	}
	if (len & 1) len++;
	len >>= 1;
	for (int i = 0; i < 8 - len; i++) {
		dst[i] = dst[i + len];
	}
	dst += 8 - len;
	for (int i = 0; i < 16; i++) {
		int c = *(src + i);
		if 		(c >= '0' && c <= '9')	c -= '0';
		else if (c >= 'a' && c <= 'f')	c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')	c -= 'A' - 10;
		else break;

		if (i % 2 == 1) dst[i / 2 % 8] = (dst[i / 2 % 8] & 0b11110000) | c;
		else 			dst[i / 2 % 8] = (dst[i / 2 % 8] & 0b1111)     | (c << 4);
	}
	return len;
}

// 表示を初期化して、srcをdestに変換
void decode2(char* src, char* dst) {
	for (int j = 0; j < 8; j++) {
		dst[j] = 0;
	}
	for (int i = 0; i < 16; i++) {
		int c = *(src + i);
		if 		(c >= '0' && c <= '9') c -= '0';
		else if (c >= 'a' && c <= 'f') c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F') c -= 'A' - 10;
		else break;
		
		for (int j = 0; j < 4; j++) {
			if (c & (1 << j)) dst[(3 - j) + i % 2 * 4] |= 1 << (i / 2 % 4 + i / 8 * 4);
		}
	}
}

/*
boolean bitman() {
	char data[16];
	decode2((char*)"0098e41f1fe49800", data);
	decode2((char*)"0884e43e3ee48408", data + 8);
	
	int ptn = 0;
	for (int i = 0; i < 100 * 6; i++) {
		setMatrix((char*)(data + ptn * 8));
		if (i % 100 == 99) ptn = 1 - ptn;
		if (uart0_test()) return 0;
	}
	return 1;
}

void bitman2() {
	char data[16];
	decode2("0098e41f1fe49800", data);
	decode2("0884e43e3ee48408", data + 8);
	
	int ptn = 0;
	playMML("CDE2CDE2");
	for (;;) {
		setMatrix2((char*)(data + ptn * 8));
		wait(10000);
		ptn = 1 - ptn;
		
		if (ux_btn()) ptn = 1 - ptn;
	}
}
*/

/*
void test() {
	// test
	for (;;) {
		for (int i = 0; i < 8 * 8; i++) {
			matrixled_on(i % 8, i / 8);
#if SYSTICK_WAIT == 1
			wait(1);
#endif
		}
	}
}
*/

void init_frame() {
	for (int i = 0; i < N_FRAME; i++) {
		g_fr->waitms[i] = 0;
		memset(g_fr->frame[i], 0, 8);
	}
}

void loadFlash(char* buf, int len) {
	for (int i = 0; i < len; i++)
		buf[i] = SAVED_FLASH[i];
}

boolean load() {
	g_fr = (struct Frame*)(g_data + 8);
	loadFlash(g_data, LEN_DATA);
	if (startsWith(g_data, (char *)"MATLED00")) {
		println("MATLED00");
		if (g_fr->waitms[0] == 0)
			return false;
		return true;
	}
	memcpy(g_data, "MATLED00", 8);
	init_frame();
	return false;
}
void save() {
	saveFlash(g_data, LEN_DATA);
}

/*
MATLED SET n data wait
	n 0-100, data:nnx8 wait(msec)
MATLED RUN

MATLED SHOW FFFFFFFFFFFFFFFF
MATLED SHOW 55aa55aa55aa55aa
MATLED SHOW 183C7EFFFF7E3C18

00011000
00111100
01111110
11111111
11111111
01111110
00111100
00011000
*/

void slowClock() {
	// watchdog
	SYSAHBCLKCTRL |= AHB_WDT; // watch dog timer power on
	
	// (1)0.6MHz / 64 = 9.3kHz 誤差40%
	// (f)4.6MHz / 64 = 71kHz
	// (f)4.6MHz / 2 = 2.3MHz 誤差40%
	int freqsel = 0xf; // 0:analog? 0x1:0.6MHz - 0xf:4.6Mhz
	int divsel = 2; // 2 - 64
	WDTOSCCTRL = (divsel / 2 - 1) | (freqsel << 5);
	
	PDRUNCFG &= ~(1 << 6); // And powered Watchdog oscillator in the Power-down configuration register (PDRUNCFG).
	
	WDTCLKSEL  = 2; // WDT = watch dog oscillator
	WDTCLKUEN  = 0;
	WDTCLKUEN  = 1;
	
	MAINCLKSEL = 2; // main clock = WDT
	MAINCLKUEN = 0;
	MAINCLKUEN = 1;
}

void deepPowerDown() {
	PCON |= 0b10;
	SCR  |= 0b100;
	PDRUNCFG |= 0b11; // IRCOUT_PD IRC_PD
	asm("wfi");
}

void sleep_tick() {
	if (ux_state())	g_timenobtn = 0;
	g_timenobtn++;
	if (g_timenobtn > 20 * 10000) deepPowerDown();
}

char g_disp_buf[8];
#define PSET(x,y) g_disp_buf[x & 7] |= 1 << (y & 7)
#define PRESET(x,y) g_disp_buf[x & 7] &= ~(1 << (y & 7))
#define CLS(n) for (int i = 0; i < 8; i++) g_disp_buf[i] = n ? 0xff : 0;
#define FILL(l) decode2(l, g_disp_buf)
#define FLUSH() setMatrix2(g_disp_buf)
	
#define PTN_3  "0038040438040438" // 3
#define PTN_2  "003804040810203c" // 2
#define PTN_1  "001828080808083c" // 1
#define PTN_GO "0073958585b59576" // GO
	
char* PTN_NUM[] = {
	"708898a8c8887000", // 0
	"2060a0202020f800", // 1
	"7088080830c0f800", // 2
	"7088083008887000", // 3
	"3050909090f81000", // 4
	"f880f00808887000", // 5
	"7080f08888887000", // 6
	"f888081010202000", // 7
	"7088887088887000", // 8
	"7088888878087000", // 9
	"70888888f8888800", // A
	"f04848704848f000", // B
	"7088808080887000", // C
	"f04848484848f000", // D
	"f88080f88080f800", // E
	"f88080f880808000", // F
	"7088808098887800", // G
	"888888f888888800", // H
	"7020202020207000", // I
	"3810101010906000", // J
	"8890a0c0a0908800", // K
	"808080808080f800", // L
	"88d8d8a8a8a88800", // M
	"88c8c8a898988800", // N
	"7088888888887000", // O
	"f0888888f0808000"  // P
/*
	70888888a8906800 // Q
	f0888888f0908800 // R
	7088807008887000 // S
	f820202020202000 // T
	8888888888887000 // U
	8888505050202000 // V
	8888a8a8a8505000 // W
	8888502050888800 // X
	8888502020202000 // Y
	f80810204080f800 // Z
	7040404040407000 // [
	8850f820f8202000 // ¥
	7010101010107000 // ]
	2050880000000000 // ^
	000000000000f800 // _
	4020100000000000 // `
	0000700878887400 // a
	8080b0c88888f000 // b
	0000708880887000 // c
	0808689888887800 // d
	00007088f8807000 // e
*/
};
	

void uart() {
#define SIZE_BUF 128
	char input_buf[SIZE_BUF];	// uartからのコマンド待ち状態
	int nbuf = 0;		// uartからの入力文字数
	int mode = 0;		// 0: 停止状態 1: RUN状態
	int cnt = 0;		// RUN時のwaitカウンタ
	int nframe = 0;		// 実行中のフレーム番号
	char data[8];		// 表示バッファ

	for (int i = 0; i < SIZE_BUF; i++)
		input_buf[i] = 0;
	for (int i = 0; i < 8; i++)
		data[i] = 0;

	CLS(1);
	
	for (int i = 0;; i++) {
		// deepSleepしない
		g_timenobtn = 0;
		while (uart0_test()) {
			int c = uart0_getc();
			if (c == '\n') {
				input_buf[nbuf] = '\0';
				if (startsWith(input_buf, (char *)"MATLED SHOW ")) {
					/* 16進数入力で複数列を表示
					 MATLED SHOW FFFFFFFFFFFFFFFF
					 入力された列数分、表示中の列を出力
					 最大8列分更新
					*/
					int flow_data[8];
					int len; 
					// 表示データを退避
					for (int j = 0 ; j < 8 ; j++) flow_data[j] = (int)data[j];

					// 表示の更新
					len = decode(input_buf + 12, data);

					// 次のマトリクスに送るデータ
					xprintf("MATLED SHOW ");
					for (int j = 0 ; j < len ; j++) xprintf("%02X", flow_data[j]);
					xprintf("\n");
				} else if (startsWith(input_buf, "MATLED PUSH ")) {
					char hex[3];
					int n = parseInt(input_buf + 12);
					int n1 = (n >> 4) & 0xf;
					int n2 = n & 0xf;
					hex[0] = n1 <= 9 ? '0' + n1 : 'A' + n1 - 10;
					hex[1] = n2 <= 9 ? '0' + n2 : 'A' + n2 - 10;
					hex[2] = 0;

					// 表示データを退避
					int flow_data = (int)data[0];

					// 表示の更新
					decode(hex, data);

					// 次のマトリクスに送るデータ
					xprintf("MATLED PUSH %d\n", flow_data);
				} else if (startsWith(input_buf, "MATLED SET ")) {
					char* pbuf = input_buf + 11;
					int nf = parseInt(pbuf);
					if (nf >= 0 && nf <= N_FRAME) {
						int n = indexOf(pbuf, ' ');
						if (n >= 0) {
							pbuf += n + 1;
							decode2(pbuf, g_fr->frame[nf]);
							decode2(pbuf, data); // 停止時の画面にも表示
							n = indexOf(pbuf, ' ');
							int nw = 100;
							if (n >= 0) {
								pbuf += n + 1;
								nw = parseInt(pbuf);
							}
							g_fr->waitms[nf] = nw;
						}
					}
				} else if (startsWith(input_buf, "MATLED CLEAR")) {
					mode = 0;
					init_frame();
				} else if (startsWith(input_buf, "MATLED RUN")) {
					mode = 1;
					println("RUN");
				} else if (startsWith(input_buf, "MATLED STOP")) {
					mode = 0;
					println("STOP");
				} else if (startsWith(input_buf, "MATLED SAVE")) {
					save();
					println("SAVE");
				} else if (startsWith(input_buf, "MATLED LOAD")) {
					load();
					println("LOAD");
				}
				nbuf = 0;
				continue;
			} else if (c == '\r') {
			} else {
				if (nbuf < SIZE_BUF - 1) input_buf[nbuf++] = c;
			}
		}
		if (mode == 0) {
			for (int k = 0; k < 8; k++) g_disp_buf[k] = data[k];
			FLUSH();
		} else {
			setMatrix(g_fr->frame[nframe]);
			
			cnt++;
			if (cnt >= g_fr->waitms[nframe]) {
				cnt = 0;
				int bknframe = nframe;
				for (;;) {
					nframe++;
					if (nframe == N_FRAME)		nframe = 0;
					if (g_fr->waitms[nframe])	break;
					if (bknframe == nframe) {
						mode = 0;
						break;
					}
				}
			}
		}
	}
}

// buf
void matrix_put(char* s, int dx, int dy, int dw, int dh) {
	char src[8];
	decode2(s, src);
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

// おみくじ
void app_mikuji() {
	for (;;) {
		playMML("C8G8C8>G4");
		ux_btn();
		for (;;) {
			FILL("c152f4d2014a4530"); // title
			FLUSH();
			if (ux_btn())
				break;
			rnd();
		}
		g_systick = 0;
		for (;;) {
			WAIT(10);
			if (!ux_state())
				break;
			if (g_systick > 10000)
				return;
		}
		
		// 00494bef4da9af00 大凶
		int btn = 0;
		g_systick = 0;
		// 大中小末
		char* PTN_UP[] = {
					"00494bef4da9af00", // 大
					"0049ebefed494f00", // 中
					"00464f46ef594f00", // 小
					"00e64fe64fe9df00", // 末
				};
		char* PTN_DOWN[] = {
					"0060f060f090f000", // 吉
					"0090b0f0d090f000", // 凶
				};
		playMML("G8");
		for (int k = 0; k < 8; k++) {
			CLS(0);
			matrix_put("c152f4d2014a4530", 0, -k, 8, 8);
			matrix_put("00494bef4da9af00", 0, -k + 8, 8, 8);
			FLUSH();
			WAIT(2000 - k * 20);
		}
		int view = 0;
		int next = rnd() % 4;
		int view2 = 0;
		int next2 = rnd() % 2;
		int state = 0;
		int wcnt = 15;
		int wcnt2 = 15;
		int i = 0;
		int j = 0;
		int ccnt = 0;
		int ccnt2 = 0;
		ux_btn();
		for (;;) {
			CLS(0);
			matrix_put(PTN_UP[view], 0, -(i % 8), 4, 8); // 大
			matrix_put(PTN_UP[next], 0, 8 - i % 8, 4, 8); // 中
			matrix_put(PTN_DOWN[view2], 4, -(j % 8), 4, 8); // 吉
			matrix_put(PTN_DOWN[next2], 4, 8 - j % 8, 4, 8); // 吉
			FLUSH();
			WAIT(1);
			if (!btn) {
				if (ux_btn()) {
					playMML("A8");
					btn = 1;
				}
			}
			if (state == 0) {
				ccnt++;
				if (ccnt == wcnt) {
					i++;
					ccnt = 0;
					if (i % 8 == 0) {
						playMML("C16");
						view = next;
						int n = rnd() % 6;
						next = n > 3 ? 0 : n;
						if (btn) {
							wcnt += wcnt;
							if (wcnt > 100) {
								state++;
								btn = 0;
							}
						}
					}
				}
			}
			ccnt2++;
			if (ccnt2 == wcnt2) {
				j++;
				ccnt2 = 0;
				if (j % 8 == 0) {
					if (state == 1)
						playMML("C16");
					view2 = next2;
					next2 = rnd() % 4 == 0 ? 1 : 0;
					if (state == 1) {
						if (btn) {
							wcnt2 += wcnt2;
							if (wcnt2 > 100)
								break;
						}
					}
				}
			}
		}
		if (view == 0 && view2 == 0) playMML("G16R8G2");
		else if (view2 == 1) 		 playMML("C2C8C8");
		else 						 playMML("C8E8G8");

		ux_btn();
		for (;;) {
			matrix_put(PTN_UP[view], 0, -(i % 8), 4, 8); // 大
			matrix_put(PTN_DOWN[view2], 4, 0, 4, 8); // 吉
			FLUSH();
			WAIT(10);
			if (ux_btn()) break;
		}
	}
}

/*
 10秒あてゲーム
 Powered by 後田宏さん
*/
void app_hit10() {
	for (;;) {
		playMML("L8ER8EG16E16");
		ux_btn();
		for (;;) {
			FILL("afeaaa0067252577"); // title
			FLUSH();
			if (ux_btn()) break;
		}
		g_systick = 0;
		for (;;) {
			WAIT(10);
			if (!ux_state()) break;
			if (g_systick > 10000) return;
		}
		playMML("C");
		FILL(PTN_3);
		FLUSH();
		WAIT(1000);

		playMML("C");
		FILL(PTN_2);
		FLUSH();
		WAIT(1000);
		
		playMML("C");
		FILL(PTN_1);
		FLUSH();
		WAIT(1000);
		
		playMML("G2");
		FILL(PTN_GO);
		FLUSH();
		WAIT(1000);
		
		CLS(1);
		FILL("043E2F566AD6AC78"); // title(Ichigo)
		FLUSH();
		
		g_systick = 0;
		int bkbtn = 0;
		for (;;) {
			int btn = ux_btn();
			if (btn && !bkbtn) {
				playMML("A4");
				CLS(0);
				break;
			}
			bkbtn = btn;
			FLUSH();
			wait(10);
		}
		int score = (100 - (g_systick/1080));

		if (score < 0) score = -score;
		playMML("L8CEG");
		FILL("00c9aaacacaaaa69"); // ok

		xprintf("%d\n", g_systick);
		xprintf("%d\n", score);

		FILL(PTN_NUM[score / 10]);
		PSET(6, 6);
		FLUSH();
		WAIT(1000);
		
		FILL(PTN_NUM[score % 10]);
		FLUSH();
		WAIT(1000);

		FLUSH();
		WAIT(1000);
	}
}

// 連打ゲーム
void app_renda() {
	for (;;) {
		playMML("L8EGG");
		ux_btn();
		for (;;) {
			FILL("8aa2cc006595f010"); // title
			FLUSH();
			if (ux_btn()) break;
		}
		g_systick = 0;
		for (;;) {
			WAIT(10);
			if (!ux_state()) break;
			if (g_systick > 10000) return;
		}
		playMML("C");
		FILL(PTN_3);
		FLUSH();
		WAIT(1000);

		playMML("C");
		FILL(PTN_2);
		FLUSH();
		WAIT(1000);
		
		playMML("C");
		FILL(PTN_1);
		FLUSH();
		WAIT(1000);
		
		playMML("G2");
		FILL(PTN_GO);
		FLUSH();
		WAIT(1000);
		
		CLS(1);
		FLUSH();
		
		g_systick = 0;
		int cnt = 0;
		ux_btn();
		for (;;) {
			int btn = ux_btn();
			if (btn) {
				playMML("A16");
				PRESET(cnt % 8, cnt / 8);
				FLUSH();
				cnt++;
				if (cnt == 64) break;
			}
		}
		playMML("L8CEG");
		FILL("00c9aaacacaaaa69"); // ok
		xprintf("%d\n", g_systick);
		unsigned int score = 100000 / (g_systick / 64);
		xprintf("%d\n", score);

		if (score > 250) score = 250;
		FILL(PTN_NUM[score / 10]);
		PSET(6, 6);
		FLUSH();
		WAIT(1000);

		FILL(PTN_NUM[score % 10]);
		FLUSH();
		WAIT(1000);

		FLUSH();
		WAIT(1000);
	}
}

// アニメーション表示
boolean animate(const unsigned char* data, int len) {
	char c;	// 数珠つなぎ対応

	for (int loop = 0; loop < 10; loop++) {
		for (int i = 0; i < len - 8; i++) {
			g_timenobtn = 0;
			c = *g_disp_buf;	// 次のMATLEDに送るデータ

			CLS(1);
			for (int k = 0; k < 8; k++) g_disp_buf[k] = data[k + i];
			FLUSH();
			xprintf("MATLED SHOW %02X\n", (int)c);

			// 90ms待ち
			for (int j = 0; j < 90; j++) {
				WAIT(1);
				// アニメーション中にボタンが押されたら次のミニゲーム
				if (ux_btn()) {
					return 1;
				}
				// アニメーション中にUART入力が有れば、MATLEDモード
				if (uart0_test()) {
					return 0;
				}
			}
		}
	}
	deepPowerDown();
	return 1;
}

int main() {
	matrixled_init();
	ux_init();
	
#if ENEBLE_WDT == 1
	slowClock();
#endif
	
#if SYSTICK_WAIT == 1
#if ENEBLE_WDT == 1
	InitSysTick(120000);
#else
	InitSysTick(12000);	// 12,000,000Hz 12,000 -> 10 = 1ms
#endif
#endif
	
#if ENEBLE_WDT == 0
	initUART();
#endif
	for (;;) {
		if (!ux_state()) break;
		WAIT(10);
	}
	for (;;) {
		if (animate(DATA_ANIM, LEN_DATA_ANIM) == 0) uart();
		app_mikuji();
		app_renda();
		app_hit10();
	}
	return 0;
}
