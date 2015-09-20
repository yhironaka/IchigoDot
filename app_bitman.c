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
