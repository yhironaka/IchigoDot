/***************************************************
** app_bitman.c
** bitman アニメーション　アプリ
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
#include "apps.h"
#include "psg.h"
#include "matrixled.h"

void app_bitman() {
	char data[16];
	decode_left2right("0098e41f1fe49800", data);
	decode_left2right("0884e43e3ee48408", data + 8);
	int ptn = 0;
	int exit_triger = 0;
	int exit_count = 0;

	set_matrix(data);
	playMML("CDE2CDE2");
	WAIT(1000);
	set_systick(0);
	for (;;) {
		wait(1);
		set_matrix(data + ptn * 8);
		if (get_systick() > 10000) {
			ptn = 1 - ptn;	// パターン切り替え
			set_systick(0);	
		}
		if (ux_state()) {
			if (exit_count == 3) return;
			if (exit_triger == get_systick()) exit_count++;
			if (exit_triger == 0) {
				playMML("C8");
				exit_triger = get_systick();
			}
		}
		if (!ux_state()) {
			exit_triger = 0;
		}
	}
}
