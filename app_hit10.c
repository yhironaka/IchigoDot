/***************************************************
** app_hit10.c
** Hit10 Game Application 10秒あてゲーム　アプリ
** Powered by 後田宏さん
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

void app_hit10() {
	for (;;) {
		playMML("L8ER8EG16E16");
		ux_btn();
		for (;;) {
			FILL("afeaaa0067252577"); // title
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
		FILL("043E2F566AD6AC78"); // title(Ichigo)
		FLUSH();
		
		set_systick(0);
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
		int score = (100 - (get_systick()/1080));

		if (score < 0) score = -score;
		playMML("L8CEG");
		FILL("00c9aaacacaaaa69"); // ok

		xprintf("%d\n", get_systick());
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
