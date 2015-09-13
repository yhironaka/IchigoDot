//  WAREABLE MATRIX min
//    License: CC BY http://fukuno.jig.jp/


#define TYPE 0 // 通常
//#define TYPE 1 // マトリクスLEDの位置、間違えた時

#define AUTO_SLEEP 1
#define PSG_ON 1

#define ROTATE 2
//#define ROTATE 1 // 右回転
//#define ROTATE 3 // 左回転
//#define ROTATE 2 // 反転

#define ENEBLE_WDT 0 // 0:12MHz .. 115200bpsでシリアル通信不可 57600bpsならok,  1:2.3MHz 消費電流最小（これより遅いとちらつく、シリアル通信不可）

/*
LPC1100L
	50MHz：7mA
	12MHz：2mA
	1MHz：840uA
	2.3MHz: 1.93mA ・・・WDTを使った最大クロック 1MHzからリニアと仮定、ほとんど12MHzと変わらない SYSTICK止めればもうちょっといい？
	9.3kHz: どうなる？？ WDTを使った最小クロック
*/

//#define SYSTICK_WAIT (!ENEBLE_WDT)
#define SYSTICK_WAIT 1


#include "LPC1100.h"
#include "uart.h"
#include "xprintf.h"
#include <string.h>
#include "rnd.h"

// util
#define boolean unsigned char
#define true 1
#define false 0

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


//
boolean ux_state() {
	return GPIO1MASKED[1 << 4] == 0;
}
int uxbtn = 0;
int bkuxbtn = 0;
void ux_tick() {
	int btn = ux_state();
	if (btn && !bkuxbtn) {
		uxbtn = 1;
	}
	bkuxbtn = btn;
}

boolean ux_btn() {
	//	return GPIO1MASKED[1 << 4] == 0;
	if (uxbtn) {
		uxbtn = 0;
		return 1;
	}
	return 0;
}
/*
void sound(int tone, int usec) {
	__set_SYSAHBCLKCTRL(PCCT16B0, 1); // on 16bit timer 0
	TMR16B0PR  = (SYSCLK / 1000000) - 1; // pre scaler (16bit) 小さすぎると間に合わなくて変わらない状態になる
	TMR16B0MR0 = usec;
//	TMR16B0MR1 = usec * 100;
	TMR16B0MCR = 0b011; // setting MR0 設定 stop reset interrupt
//	TMR16B0EMR = 0x3; // toggle MAT0
	__enable_irqn(CT16B0_IRQn);
	__set_irqn_priority(CT16B0_IRQn, 2);
	TMR16B0TCR = 1;
}
void CT16B0_IRQHandler(void) {
	if (TMR16B0IR & 0x01) {
		GPIO1MASKED[1 << 5] = ~GPIO1MASKED[1 << 5];
		TMR16B0IR = 0x01;
	}
}
*/

#include "psg.h"

void toggleSounder() {
	GPIO1MASKED[1 << 5] = ~GPIO1MASKED[1 << 5];
}

/*
void psg_tick() {
	if (ux_btn())
		toggleSounder();
}
*/

void ux_init() {
	IOCON_PIO1_4 = 0x000000d0;
	GPIO1DIR &= ~(1 << 4);
	IOCON_PIO1_5 = 0x000000d0;
	GPIO1DIR |= 1 << 5;
	
	psg_init();
}


#if TYPE==0
const char xpos[] = { 4, 2, 109, 3, 100, 108, 5, 103 };
const char ypos[] = { 6, 8, 9, 102, 7, 101, 11, 10 };
#endif

#if TYPE==1
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
const char xpos[] = { 6, 102, 5, 103, 9, 10, 4, 3 };
const char ypos[] = { 7, 101, 100, 2, 11, 8, 109, 108 };
#endif

int timenobtn = 0;

short d0def = 0;
short d1def = 0;
short d0mask = 0;
short d1mask = 0;

char matbuf[8];
char matcnt;

void matrixled_init() {
	matcnt = 0;
	for (int i = 0; i < 8; i++)
		matbuf[i] = 0;
	
	IOCON_PIO0_7 = 0x000000d0;
	GPIO0DIR |= 1 << 7;
	IOCON_PIO0_4 = 0x000000d0;
	GPIO0DIR |= 1 << 4;
	IOCON_PIO0_3 = 0x000000d0;
	GPIO0DIR |= 1 << 3;
	IOCON_PIO0_2 = 0x000000d0;
	GPIO0DIR |= 1 << 2;
	IOCON_PIO1_9 = 0x000000d0;
	GPIO1DIR |= 1 << 9;
	IOCON_PIO1_8 = 0x000000d0;
	GPIO1DIR |= 1 << 8;
	
	IOCON_PIO0_8 = 0x000000d0;
	GPIO0DIR |= 1 << 8;
	IOCON_PIO0_9 = 0x000000d0;
	GPIO0DIR |= 1 << 9;
	IOCON_SWCLK_PIO0_10 = 0x000000d1;
	GPIO0DIR |= 1 << 10;
	IOCON_R_PIO0_11 = 0x000000d1;
	GPIO0DIR |= 1 << 11;
	IOCON_PIO0_5 = 0x000000d0;
	GPIO0DIR |= 1 << 5;
	IOCON_PIO0_6 = 0x000000d0;
	GPIO0DIR |= 1 << 6;
	IOCON_R_PIO1_0 = 0x000000d1;
	GPIO1DIR |= 1 << 0;
	IOCON_R_PIO1_1 = 0x000000d1;
	GPIO1DIR |= 1 << 1;
	IOCON_R_PIO1_2 = 0x000000d1;
	GPIO1DIR |= 1 << 2;
	IOCON_SWDIO_PIO1_3 = 0x000000d1;
	GPIO1DIR |= 1 << 3;
	
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
		int nx = xpos[i];
		if (nx < 100) {
			d0def |= 1 << nx;
			d0mask |= 1 << nx;
		} else {
			d1def |= 1 << (nx - 100);
			d1mask |= 1 << (nx - 100);
		}
		int ny = ypos[i];
		if (ny < 100) {
			d0mask |= 1 << ny;
		} else {
			d1mask |= 1 << (ny - 100);
		}
	}
	GPIO0MASKED[d0mask] = d0def;
	GPIO1MASKED[d1mask] = d1def;
//	GPIO0DATA = d0def;
//	GPIO1DATA = d1def;
//	GPIO0MASKED[0b000000111100] = 0x7ff;
//	GPIO1MASKED[0b001100001001] = 0x7ff;
	
	// 0_5 つかない
	//            BA9876543210
	/*
	GPIO0DATA = 0b010000111100;
	GPIO1DATA = 0b001100000001;
	for (;;);
	*/
}
void matrixled_on(int x, int y) {
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x > 7)
		x = 7;
	if (y > 7)
		y = 7;
	
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
	
	int d0 = d0def; // 0b000000111100;
	int d1 = d1def; // 0b001100001001;
	int ny = ypos[y];
	if (ny < 100) {
		d0 |= 1 << ny;
	} else {
		d1 |= 1 << (ny - 100);
	}
	int nx = xpos[x];
	if (nx < 100) {
		d0 &= ~(1 << nx);
	} else {
		d1 &= ~(1 << (nx - 100));
	}
//	GPIO0DATA = d0;
//	GPIO1DATA = d1;
	GPIO0MASKED[d0mask] = d0;
	GPIO1MASKED[d1mask] = d1;
}
void matrixled_off() {
	GPIO0MASKED[d0mask] = d0def;
	GPIO1MASKED[d1mask] = d1def;
	
//	GPIO0DATA = d0def;
//	GPIO1DATA = d1def;
//	GPIO0DATA = 0b000000111100;
//	GPIO1DATA = 0b001100001001;
}
void matrixled_tick() {
	int j = matcnt / 8;
	int i = matcnt % 8;
	char d = matbuf[j];
	if (d & (1 << i)) {
		matrixled_on(i, j);
	} else {
		matrixled_off();
	}
	matcnt++;
	if (matcnt == 64)
		matcnt = 0;
}

// uart

void initUART() {
	uart0_init();
	xdev_out(uart0_putc);
	xdev_in(uart0_getc);
}

void sleep_tick();

/*
systick
*/
volatile int systick;
void InitSysTick(int hz) {
	SYST_RVR = SYSCLK / hz - 1;
	SYST_CSR = 0x07;
}
void SysTick_Handler(void) {
	systick++;
	//
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
	int endt = systick + n;
	for (;;) {
		if (systick > endt)
			break;
	}
}

// util
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

// timer

// timer
/*
void startTimer16(int usec) {
	
	__set_SYSAHBCLKCTRL(PCCT16B1, 1); // on 16bit timer 0
//	TMR16B1PR  = (SYSCLK / 1000) - 1; // pre scaler (16bit)
//	TMR16B1MR0 = msec;
	TMR16B1PR  = (SYSCLK / 1000000) - 1; // pre scaler (16bit) 小さすぎると間に合わなくて変わらない状態になる
	TMR16B1MR0 = usec;
	TMR16B1MCR = 0b011; // setting MR0 設定 stop reset interrupt
	__enable_irqn(CT16B1_IRQn);
	__set_irqn_priority(CT16B1_IRQn, 2);
	TMR16B1TCR = 1;
}
void CT16B1_IRQHandler(void) {
	if (TMR16B1IR & 0x01) {
		// todo
		TMR16B1IR = 0x01;
	}
}
*/
void setMatrix2(char* data) {
	for (int j = 0; j < 8; j++) {
		matbuf[j] = data[j];
	}
}
void setMatrixLong(long data) {
	setMatrix2((char*)&data);
}

// urt

/*
0123456789abcdef
ff181818181818ff
*/



// bitman

/* bitman
0098e41f1fe49800
0884e43e3ee48408
*/
void decode(unsigned char* src, unsigned char* dst) {
	int len = 0;
	for (int i = 0; i < 16; i++) {
		int c = *(src + i);
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')
			c -= 'A' - 10;
		else
			break;
		len++;
	}
	if (len & 1)
		len++;
	len >>= 1;
	for (int i = 0; i < 8 - len; i++) {
		dst[i] = dst[i + len];
	}
	dst += 8 - len;
	for (int i = 0; i < 16; i++) {
		int c = *(src + i);
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')
			c -= 'A' - 10;
		else
			break;
		if (i % 2 == 1) {
			dst[i / 2 % 8] = (dst[i / 2 % 8] & 0b11110000) | c;
		} else {
			dst[i / 2 % 8] = (dst[i / 2 % 8] & 0b1111) | (c << 4);
		}
	}
}
void decode2(unsigned char* src, unsigned char* dst) {
	for (int j = 0; j < 8; j++) {
		dst[j] = 0;
	}
	for (int i = 0; i < 16; i++) {
		int c = *(src + i);
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')
			c -= 'A' - 10;
		else
			break;
		
		for (int j = 0; j < 4; j++) {
			if (c & (1 << j))
				dst[(3 - j) + i % 2 * 4] |= 1 << (i / 2 % 4 + i / 8 * 4);
		}
	}
}

void setMatrixDecode(char* data) {
	decode((unsigned char*)data, (unsigned char*)matbuf);
}

boolean bitman() {
	unsigned char data[16];
	decode((unsigned char*)"0098e41f1fe49800", data);
	decode((unsigned char*)"0884e43e3ee48408", data + 8);
	
	int ptn = 0;
	for (int i = 0; i < 100 * 6; i++) {
		setMatrix((char*)(data + ptn * 8));
		if (i % 100 == 99)
			ptn = 1 - ptn;
		
		if (uart0_test())
			return 0;
	}
	return 1;
}
void bitman2() {
	unsigned char data[16];
	decode((unsigned char*)"0098e41f1fe49800", data);
	decode((unsigned char*)"0884e43e3ee48408", data + 8);
	
	int ptn = 0;
	playMML("CDE2CDE2");
	for (;;) {
		setMatrix2((char*)(data + ptn * 8));
		wait(10000);
		ptn = 1 - ptn;
		
		if (ux_btn()) {
			ptn = 1 - ptn;
		}
	}
}


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

//#define LEN_DATA 2048
#define LEN_DATA 1024
char data[LEN_DATA];

#define N_FRAME ((LEN_DATA - 8) / 10)
struct Frame {
	char frame[N_FRAME][8];
	short waitms[N_FRAME];
};
struct Frame* fr;

void init_frame() {
	for (int i = 0; i < N_FRAME; i++) {
		fr->waitms[i] = 0;
		memset(fr->frame[i], 0, 8);
	}
	/*
	*(long*)fr->frame[0] = (long)0x55aa55aa55aa55aa;
	*(long*)fr->frame[1] = (long)0xaa55aa55aa55aa55;
//	decode("0098e41f1fe49800", frame[0]);
	fr->waitms[0] = fr->waitms[1] = 100;
	*/
}

#include "iap.h"
// len 512, 1024, 2048, 4096
//void saveFlash(char* buf, int len)

void loadFlash(char* buf, int len) {
	for (int i = 0; i < len; i++)
		buf[i] = SAVED_FLASH[i];
}
boolean load() {
	fr = (struct Frame*)(data + 8);
	loadFlash(data, LEN_DATA);
	if (startsWith(data, "MATLED00")) {
		println("MATLED00");
		if (fr->waitms[0] == 0)
			return false;
		return true;
	}
	memcpy(data, "MATLED00", 8);
	init_frame();
	return false;
}
void save() {
	saveFlash(data, LEN_DATA);
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
	
	WDTCLKSEL = 2; // WDT = watch dog oscillator
	WDTCLKUEN = 0;
	WDTCLKUEN = 1;
	
	MAINCLKSEL = 2; // main clock = WDT
	MAINCLKUEN = 0;
	MAINCLKUEN = 1;
}

void deepPowerDown() {
	PCON |= 0b10;
	SCR |= 0b100;
	PDRUNCFG |= 0b11; // IRCOUT_PD IRC_PD
	asm("wfi");
}

void sleep_tick() {
	if (ux_state())
		timenobtn = 0;
	timenobtn++;
	if (timenobtn > 20 * 10000)
		deepPowerDown();
}

char dispBuf[8];
#define PSET(x,y) dispBuf[x & 7] |= 1 << (y & 7)
#define PRESET(x,y) dispBuf[x & 7] &= ~(1 << (y & 7))
#define CLS(n) for (int i = 0; i < 8; i++) dispBuf[i] = n ? 0xff : 0;
//#define FILL(l) *(long*)dispBuf = l // ng
#define FILL(l) decode2(l, (unsigned char*)dispBuf)
#define FLUSH() setMatrix2(dispBuf)
	
#define PTN_3 "0038040438040438"
#define PTN_2 "003804040810203c"
#define PTN_1 "001828080808083c"
#define PTN_GO "0073958585b59576"
	
char* PTN_NUM[] = {
	"708898a8c8887000",
	"2060a0202020f800",
	"7088080830c0f800",
	"7088083008887000",
	"3050909090f81000",
	"f880f00808887000",
	"7080f08888887000",
	"f888081010202000",
	"7088887088887000",
	"7088888878087000",
	"70888888f8888800",
	"f04848704848f000",
	"7088808080887000",
	"f04848484848f000",
	"f88080f88080f800",
	"f88080f880808000",
	"7088808098887800",
	"888888f888888800",
	"7020202020207000",
	"3810101010906000",
	"8890a0c0a0908800",
	"808080808080f800",
	"88d8d8a8a8a88800",
	"88c8c8a898988800",
	"7088888888887000",
	"f0888888f0808000"
/*
	70888888a8906800
	f0888888f0908800
	7088807008887000
	f820202020202000
	8888888888887000
	8888505050202000
	8888a8a8a8505000
	8888502050888800
	8888502020202000
	f80810204080f800
	7040404040407000
	8850f820f8202000
	7010101010107000
	2050880000000000
	000000000000f800
	4020100000000000
	0000700878887400
	8080b0c88888f000
	0000708880887000
	0808689888887800
	00007088f8807000
*/
};
	
#define WAIT(n) wait(n * 10)

void uart() {
#define SIZE_BUF 128
	char inputBuf[SIZE_BUF];
	int nbuf = 0;
	for (int i = 0; i < SIZE_BUF; i++)
		inputBuf[i] = 0;
	
	CLS(1);
//init_frame	init_frame();
//	load(); // 初回はデータクリアいるかも？
	int mode = 0;
	
	unsigned char data[8];
	for (int i = 0; i < 8; i++)
		data[i] = 0;
	
	int n = 0;
	int cnt = 0;
	int nframe = 0;
	for (int i = 0;; i++) {
		// deepSleepしない
		timenobtn = 0;
		while (uart0_test()) {
			int c = uart0_getc();
			if (c == '\n') {
				inputBuf[nbuf] = '\0';
				if (startsWith(inputBuf, "MATLED SHOW ")) {
					char c = *data;
					decode(inputBuf + (9 + 3), data);
					xprintf("MATLED SHOW %02X\n", (int)c);
				} else if (startsWith(inputBuf, "MATLED PUSH ")) {
					char hex[3];
					int n = parseInt(inputBuf + (9 + 3));
					int n1 = (n >> 4) & 0xf;
					int n2 = n & 0xf;
					hex[0] = n1 <= 9 ? '0' + n1 : 'A' + n1 - 10;
					hex[1] = n2 <= 9 ? '0' + n2 : 'A' + n2 - 10;
					hex[2] = 0;
					decode(hex, data);
					xprintf("MATLED PUSH %s\n", hex);
				} else if (startsWith(inputBuf, "MATLED SET ")) {
					char* pbuf = inputBuf + 11;
					int nf = parseInt(pbuf);
					if (nf >= 0 && nf <= N_FRAME) {
						int n = indexOf(pbuf, ' ');
						if (n >= 0) {
							pbuf += n + 1;
//							println(pbuf);
							decode(pbuf, fr->frame[nf]);
							decode(pbuf, data); // 停止時の画面にも表示
							n = indexOf(pbuf, ' ');
							int nw = 100;
							if (n >= 0) {
								pbuf += n + 1;
								nw = parseInt(pbuf);
							}
							fr->waitms[nf] = nw;
						}
					}
				} else if (startsWith(inputBuf, "MATLED CLEAR")) {
					mode = 0;
					init_frame();
				} else if (startsWith(inputBuf, "MATLED RUN")) {
					mode = 1;
					println("RUN");
				} else if (startsWith(inputBuf, "MATLED STOP")) {
					mode = 0;
					println("STOP");
				} else if (startsWith(inputBuf, "MATLED SAVE")) {
					save();
					println("SAVE");
				} else if (startsWith(inputBuf, "MATLED LOAD")) {
					load();
					println("LOAD");
				}
				nbuf = 0;
				continue;
			} else if (c == '\r') {
			} else {
				if (nbuf < SIZE_BUF - 1)
					inputBuf[nbuf++] = c;
			}
		}
		if (mode == 0) {
			for (int k = 0; k < 8; k++)
				dispBuf[k] = data[k];
			FLUSH();
//			setMatrix(data);
		} else {
			setMatrix(fr->frame[nframe]);
			
			cnt++;
			if (cnt >= fr->waitms[nframe]) {
				cnt = 0;
				int bknframe = nframe;
				for (;;) {
					nframe++;
					if (nframe == N_FRAME)
						nframe = 0;
					if (fr->waitms[nframe])
						break;
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
	unsigned char src[8];
	decode2(s, src);
	for (int i = 0; i < dw; i++) {
		for (int j = 0; j < dh; j++) {
			int x = dx + i;
			int y = dy + j;
			if (x < 0 || y < 0 || x > 7 || y > 7)
				continue;
			PRESET(x, y);
			int n = (src[i] >> j) & 1;
			if (n) {
				PSET(x, y);
			}
		}
	}
	/*
//	dx = dy = 0;
//	dw = dh = 8;
	int mask = ((1 << dh) - 1) >> (8 - dh + dy);
	for (int i = 0; i < dw; i++) {
		int y = dx + i;
		if (y < 0 || y > 7)
			continue;
		buf[y] &= ~mask;
		buf[y] |= (src[i] >> dy) & mask;
	}
	*/
}

void app_mikuji() { // おみくじ
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
		systick = 0;
		for (;;) {
			WAIT(10);
			if (!ux_state())
				break;
			if (systick > 10000)
				return;
		}
		
		// 00494bef4da9af00 大凶
		int btn = 0;
		systick = 0;
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
//				if (systick > 10000 && ux_btn())
				//					btn = 1;
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
		if (view == 0 && view2 == 0) {
			playMML("G16R8G2");
		} else if (view2 == 1) {
			playMML("C2C8C8");
		} else {
			playMML("C8E8G8");
		}
		ux_btn();
		for (;;) {
			matrix_put(PTN_UP[view], 0, -(i % 8), 4, 8); // 大
			matrix_put(PTN_DOWN[view2], 4, 0, 4, 8); // 吉
			FLUSH();
			WAIT(10);
			if (ux_btn())
				break;
		}
		/*
		for (;;) {
			WAIT(100);
			if (!ux_btn())
				break;
		}
		*/
	}
}


void app_hit10() { // 10秒あてゲーム
	for (;;) {
		playMML("L8ER8EG16E16");
		ux_btn();
		for (;;) {
			FILL("afeaaa0067252577"); // title
			FLUSH();
			if (ux_btn())
				break;
		}
		// 長押しでゲーム切り替え
		systick = 0;
		for (;;) {
			WAIT(10);
			if (!ux_state())
				break;
			if (systick > 10000)
				return;
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
		systick = 0;
		int cnt = 0;
		int bkbtn = 0;
		for (;;) {
			int btn = ux_btn();
			if (btn && !bkbtn) {
				playMML("A4");
				CLS(0);
				break;
			}
			bkbtn = btn;
			setMatrix2(dispBuf);
			wait(10);
		}
		unsigned int score = (10 * 100000 - systick) / 1000;
		if (score < 0)
			score = -score;
		playMML("L8CEG");
		FILL("00c9aaacacaaaa69"); // ok
		xprintf("%d\n", systick);
		xprintf("%d\n", score);
		/*
		for (int i = 0;; i++) {
			int n = time % 10;
			time /= 10;
			if (time == 0)
				break;
			FILL(PTN_NUM[n]);
			FLUSH();
			WAIT(500);
		}
		*/
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

void app_keytest() {
	for (;;) {
		CLS(ux_state());
		FLUSH();
//		WAIT(10);
	}
	int flg = 0;
	for (;;) {
		if (ux_btn())
			flg = !flg;
		CLS(flg);
		FLUSH();
//		WAIT(1000); // 1,000sec
		WAIT(1000);
	}
}

void app_renda() { // 連打ゲーム
	for (;;) {
		playMML("L8EGG");
		ux_btn();
		for (;;) {
			FILL("8aa2cc006595f010"); // title
//			FILL("8aa2cc006a953060"); // title
			FLUSH();
			if (ux_btn())
				break;
		}
		systick = 0;
		for (;;) {
			WAIT(10);
			if (!ux_state())
				break;
			if (systick > 10000) 
				return;
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
		systick = 0;
		int cnt = 0;
		int bkbtn = 0;
		ux_btn();
		for (;;) {
			int btn = ux_btn();
			//			if (btn && !bkbtn) {
			if (btn) {
				playMML("A16");
				PRESET(cnt % 8, cnt / 8);
				FLUSH();
				cnt++;
				if (cnt == 64)
					break;
			}
//			bkbtn = btn;
//			wait(10);
		}
		playMML("L8CEG");
		FILL("00c9aaacacaaaa69"); // ok
		xprintf("%d\n", systick);
		unsigned int score = 100000 / (systick / 64);
		xprintf("%d\n", score);
		/*
		for (int i = 0;; i++) {
			int n = time % 10;
			time /= 10;
			if (time == 0)
				break;
			FILL(PTN_NUM[n]);
			FLUSH();
			WAIT(500);
		}
		*/
		if (score > 250)
			score = 250;
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

#include "anim_newyear.h"

boolean animate(const unsigned char* data, int len) {
	char c;	// 数珠つなぎ対応

	for (int loop = 0; loop < 10; loop++) {
		for (int i = 0; i < len - 8; i++) {
			timenobtn = 0;
			//				setMatrix2(data + i);
			c = *dispBuf;	// 次のMATLEDに送るデータ

			CLS(1);
			for (int k = 0; k < 8; k++)
				dispBuf[k] = data[k + i];
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
//		break;
	}
	deepPowerDown();
	return 1;
}

int main() {
	/*
	IOCON_PIO1_5 = 0x000000d0;
	GPIO1DIR |= 1 << 5;
	GPIO1MASKED[1 << 5] = 0;
	for (int i = 0;; i++) {
		GPIO1MASKED[1 << 5] = i;
	}
	*/
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
	
	/*
	for (;;) {
		playMML("C");
		println("TEST\n");
		toggleSounder();
		wait(10000);
	}
//	uart();
	*/
	
//	bitman();
	//	bitman2();
	for (;;) {
		if (!ux_state()) {
			break;
		}
		WAIT(10);
	}
	for (;;) {
		if (animate(DATA_ANIM, LEN_DATA_ANIM) == 0){
			uart();
		}
		app_mikuji();
//		app_keytest();
		app_renda();
		app_hit10();
	}
	return 0;
}
