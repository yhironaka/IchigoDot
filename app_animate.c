/***************************************************
** app_animate.c
** Animation Application アニメーション表示　アプリ
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
#include "utils.h"
#include "uart.h"

boolean app_animate(const char* data, int len) {
	char disp_buf[8];	// 数珠つなぎ対応

	for (int loop = 0; loop < 10; loop++) {
		for (int i = 0; i < len - 8; i++) {
			no_sleep();
			get_matrix(disp_buf);

			CLS(1);
			setMatrix2((char *)data+i);
			xprintf("MATLED SHOW %02X\n", (int)disp_buf[0]);

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
