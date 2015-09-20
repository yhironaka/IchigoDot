/***************************************************
** app_mikuji.c
** Mikuji Game Application おみくじゲーム　アプリ
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
		set_systick(0);
		for (;;) {
			WAIT(10);
			if (!ux_state())
				break;
			if (get_systick() > 10000)
				return;
		}
		
		int btn = 0;
		set_systick(0);
		// 大中小末
		char* PTN_UP[] = {
					"00494bef4da9af00", // 大凶
					"0049ebefed494f00", // 中凶
					"00464f46ef594f00", // 小凶
					"00e64fe64fe9df00", // 末凶
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
			WAIT(200 - k * 20);
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
