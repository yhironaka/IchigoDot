/***************************************************
** IchigoDotS 
** License: CC BY yhironaka@gmail.com
** 
** This program was modified by Yasuhisa Hironaka.
** Original code was written by Taisuke Fukuno.
** WAREABLE MATRIX min
** License: CC BY http://fukuno.jig.jp/
***************************************************/

#include "main.h"
#include "LPC1100.h"
#include "utils.h"
#include "uart.h"
#include "psg.h"
#include "matrixled.h"
#include "anim_ichigodot.h"
#include "apps.h"

// ----- global variables -----
int 	g_uxbtn = 0;
int 	g_uxbtnret = 0;
int 	g_bkuxbtn = 0;
int 	g_timenobtn = 0;
int		g_btncount = 0;
volatile int 	g_systick;

//チャタリング対策
#define BTN_RESET 400


int get_systick() {
	return g_systick;
}

void set_systick(int systick) {
	g_systick = systick;
}

void ux_init() {
	IOCON_PIO1_4 = 0x000000d0;
	GPIO1DIR &= ~(1 << 4);
	IOCON_PIO1_5 = 0x000000d0;
	GPIO1DIR |= 1 << 5;
	
	psg_init();
}

void ux_tick() {
		if (g_btncount > 0) {
		g_btncount--;
		return;
	}
	int btn = ux_state();
	if (btn && !g_bkuxbtn) {
		g_uxbtn = 1;
		g_uxbtnret = 1;
		g_btncount = BTN_RESET;
	}
	g_bkuxbtn = btn;
	
}

boolean ux_state() {
	return GPIO1MASKED[1 << 4] == 0;
}

boolean ux_btn() {
	if (g_uxbtnret) {
		g_uxbtnret = 0;
		return 1;
	}
	return 0;
}

void uart_init() {
	uart0_init();
	xdev_out(uart0_putc);
	xdev_in(uart0_getc);

	xprintf("IchigoDotS Ver.%s\n" , VERSION_NUM);
}

void SysTick_init(int hz) {
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

void WAIT(int n){
	wait(n * 10);
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

void no_sleep() {
	g_timenobtn = 0;
}

int main() {
	matrixled_init();
	ux_init();	
	SysTick_init(12000);	// 12,000,000Hz 12,000 -> 10 = 1ms	
	uart_init();

	for (;;) {
		if (!ux_state()) break;
		WAIT(10);
	}
	for (;;) {
		if (app_animate(DATA_ANIM, LEN_DATA_ANIM) == 0) {
			app_uart();
			continue;
		}
		app_mikuji();
		app_renda();
		app_hit10();
		app_bitman();
	}
	return 0;
}
