/***************************************************
** utils.c
** IchigoDot Utilitys
** 
** IchigoDotS 
** License: CC BY yhironaka@gmail.com
** 
** This program was modified by Yasuhisa Hironaka.
** Original code was written by Taisuke Fukuno.
** WAREABLE MATRIX min
** License: CC BY http://fukuno.jig.jp/
***************************************************/

#include "utils.h"
#include "uart.h"
#include "matrixled.h"

boolean startsWith(const char* s, const char* key) {
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

int parseInt(const char* s) {
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

int indexOf(const char* s, char c) {
	for (int i = 0;; i++) {
		if (*s == 0)
			return -1;
		if (*s == c)
			return i;
		s++;
	}
}

void println(const char* s) {
	for (;;) {
		char c = *s++;
		if (c == 0)
			break;
		uart0_putc(c);
	}
	uart0_putc('\n');
}

// 表示されている列をずらして変換
int decode(const char* src, char* dst) {
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
void decode2(const char* src, char* dst) {
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

// random
// xorshift : http://www.jstatsoft.org/v08/i14/paper。Marsaglia (July 2003). “Xorshift RNGs”
// http://hexadrive.sblo.jp/article/63660775.html
static unsigned long rndk = 123456789;
static unsigned long rndy = 362436069;
static unsigned long rndz = 521288629;
static unsigned long rndw = 88675123;

unsigned int rnd() {
	unsigned long t = rndk ^ (rndk << 11);
	rndk = rndy;
	rndy = rndz;
	rndz = rndw;
	return rndw = (rndw ^ (rndw >> 19)) ^ (t ^ (t >> 8));
}