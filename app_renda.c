/***************************************************
** app_renda.c
** Renda Game Application 連打ゲーム　アプリ
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

void app_renda() {
	for (;;) {
		playMML("L8EGG");
		ux_btn();
		for (;;) {
			FILL("8aa2cc006595f010"); // title
			FLUSH();
			if (ux_btn()) break;
		}
		set_systick(0);
		for (;;) {
			WAIT(10);
			if (!ux_state()) break;
			if (get_systick() > 10000) return;
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
		
		set_systick(0);
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
		xprintf("%d\n", get_systick());
		unsigned int score = 100000 / (get_systick() / 64);
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
