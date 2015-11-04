/***************************************************
** psg.c
** Programmable sound generator
** 
** IchigoDotS 
** License: CC BY yhironaka@gmail.com
** 
** This program was modified by Yasuhisa Hironaka.
** Original code was written by Taisuke Fukuno.
** WAREABLE MATRIX min
** License: CC BY http://fukuno.jig.jp/
***************************************************/

#include "LPC1100.h"

int soundon = 0;
char psgbuf[128];
char* psgmml;
int psgwaitcnt;
int psgwaitcnt2;
int psgoct = 3;
int psgdeflen = 4;
int sound[] = { 4545, 4290, 4049, 3822, 3607, 3405, 3214, 3033, 2863, 2702, 2551, 2407, 2272, 2145, 2024, 1911, 1803, 1702, 1607, 1516, 1431, 1351, 1275, 1203, 1136, 1072, 1012, 955, 901, 851, 803, 758, 715, 675, 637, 601, 568, 536, 506, 477, 450, 425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 238, 225, 212, 200, 189, 178, 168, 159, 150, };
int stdlen = 1000;

void enableSounder(int n) {
	soundon = n;
	if (soundon) {
		GPIO1DIR |= 1 << 5;
	} else {
		GPIO1DIR &= ~(1 << 5);
	}
}

void toggleSounder() {
	GPIO1MASKED[1 << 5] = ~GPIO1MASKED[1 << 5];
}

void CT16B0_IRQHandler(void) {
	if (TMR16B0IR & 0x01){
		if (soundon)
			toggleSounder();
		TMR16B0IR = 0x01;
	}
}

void startPSG() {
	if (!TMR16B0TCR)
		TMR16B0TCR = 1;
}

void stopPSG() {
	if (TMR16B0TCR)
		TMR16B0TCR = 0;
}

void psg_init() {
	psgbuf[0] = 0;
	psgmml = psgbuf;
	psgwaitcnt = psgwaitcnt2 = 0;
	
	IOCON_PIO1_5 = 0x000000d0;
	GPIO1DIR &= ~(1 << 5);
	
	__set_SYSAHBCLKCTRL(PCCT16B0, 1); // on 16bit timer 0
	TMR16B0PR  = (SYSCLK / 1000000) - 1; // pre scaler
	TMR16B0MR0 = 50;
	TMR16B0MCR = 0b011; // setting MR0 設定 stop reset interrupt
	__enable_irqn(CT16B0_IRQn);
	__set_irqn_priority(CT16B0_IRQn, 1);
	
	startPSG();
}

// C+D+EF+G+A+B
void psg_tick() {
	if (psgwaitcnt > 0) {
		psgwaitcnt--;
		if (psgwaitcnt == 0) {
			enableSounder(0);
		}
		return;
	}
	if (psgwaitcnt2 > 0) {
		psgwaitcnt2--;
		return;
	}
	
	for (;;) {
		int t = -1;
		char c = *psgmml++;
		if (c == 0) {
			psgmml--;
			enableSounder(0);
			return;
		} else if (c == '>') {
			psgoct++;
			continue;
		} else if (c == '<') {
			psgoct--;
			continue;
		} else if (c == 'O' || c == 'o') {
			char c2 = *psgmml++;
			psgoct = c2 - '0';
			continue;
		} else if (c == ' ') {
			continue;
		} else if (c == 'L' || c == 'l') {
			char c2 = *psgmml++;
			psgdeflen = 16 / (c2 - '0');
			if (c2 == '1') {
				c2 = *psgmml++;
				if (c2 == '6')
					psgdeflen = 1;
				else
					psgmml--;
			}
			continue;
		} else if (c == 'C' || c == 'c')
			t = 0;
		else if (c == 'D' || c == 'd')
			t = 2;
		else if (c == 'E' || c == 'e')
			t = 4;
		else if (c == 'F' || c == 'f')
			t = 5;
		else if (c == 'G' || c == 'g')
			t = 7;
		else if (c == 'A' || c == 'a')
			t = 9;
		else if (c == 'B' || c == 'b')
			t = 11;
		else if (c == 'R' || c == 'r')
			t = -1;
		else
			continue;
		int c2 = *psgmml++;
		if (c2 == '+')
			t++;
		else if (c2 == '-')
			t--;
		else
			psgmml--;
		int len = psgdeflen;
		int c3 = *psgmml++;
		if (c3 == '2')
			len = 8;
		else if (c3 == '4')
			len = 4;
		else if (c3 == '8')
			len = 2;
		else if (c3 == '1')
			len = 16;
		else if (c3 == '.')
			len += len / 2;
		else
			psgmml--;
		int c4 = *psgmml++;
		if (c4 == '.')
			len += len / 2;
		else if (c4 == '6' && len == 16)
			len = 1;
		else
			psgmml--;
		if (t >= 0)
			t += (psgoct * 12);
		//
		
		if (t == -1 || t >= 5 * 12) {
			enableSounder(0);
		//		stopTimer();
		} else {
			TMR16B0MR0 = sound[t];
			enableSounder(1);
		}
		psgwaitcnt = len * 800;
		psgwaitcnt2 = psgwaitcnt / 8;
		break;
	}
}


void playMML(char* mml) {
	for (int i = 0; i < 128; i++) {
		psgbuf[i] = mml[i];
		if (!mml[i])
			break;
	}
	psgmml = psgbuf;
	psgoct = 3;
//	int tempo = 120;
	psgdeflen = 4;
	psgwaitcnt = 0;
}

