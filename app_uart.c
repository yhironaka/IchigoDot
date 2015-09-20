/***************************************************
** app_uart.c
** controled by uart Application
** 
** IchigoDotS 
** License: CC BY yhironaka@gmail.com
** 
** This program was modified by Yasuhisa Hironaka.
** Original code was written by Taisuke Fukuno.
** WAREABLE MATRIX min
** License: CC BY http://fukuno.jig.jp/
***************************************************/

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

#include <string.h>
#include "main.h"
#include "apps.h"
#include "matrixled.h"
#include "uart.h"
#include "iap.h"

#define LEN_DATA 1024
#define N_FRAME ((LEN_DATA - 8) / 10)

const char *save_tag = "MATLED01";

// ----- struct -----
struct Frame {
	char frame[8];
	short waitms;
};

struct Save_data {
	char header[8];
	struct Frame frame[N_FRAME];
};

//struct Frame* 	fr[N_FRAME];
//char 	g_data[LEN_DATA];

void init_frame(struct Save_data *sd) {
	struct Frame *fr = sd->frame;

	memcpy(sd->header, save_tag, 8);

	for (int i = 0; i < N_FRAME; i++) {
		fr[i].waitms = 0;
		memset(fr[i].frame, 0, 8);
	}
}

void loadFlash(char* buf, int len) {
	for (int i = 0; i < len; i++) buf[i] = SAVED_FLASH[i];
}

boolean load(struct Save_data *sd) {
	loadFlash((char *)sd, LEN_DATA);
	if (startsWith(sd->header, save_tag)) {
		println(save_tag);
		return true;
	}
	init_frame(sd);
	return false;
}

void save(struct Save_data *sd) {
	saveFlash((char *) sd, LEN_DATA);
}

void show(const char *input_buf , char *data) {
	/*
	16進数入力で複数列を表示
	MATLED SHOW FFFFFFFFFFFFFFFF
	入力された列数分、表示中の列を出力
	最大8列分更新
	*/
	int flow_data[8];
	int len; 
	// 表示データを退避
	for (int j = 0 ; j < 8 ; j++) flow_data[j] = (int)data[j];

	// 表示の更新
	len = decode_left2right(input_buf + 12, data);

	// 次のマトリクスに送るデータ
	xprintf("MATLED SHOW ");
	for (int j = 0 ; j < len ; j++) xprintf("%02X", flow_data[j]);
	xprintf("\n");
}

void push(const char *input_buf , char *data) {
	char hex[3];
	int n = parseInt(input_buf + 12);
	int n1 = (n >> 4) & 0xf;
	int n2 = n & 0xf;
	hex[0] = n1 <= 9 ? '0' + n1 : 'A' + n1 - 10;
	hex[1] = n2 <= 9 ? '0' + n2 : 'A' + n2 - 10;
	hex[2] = 0;

	// 表示データを退避
	int flow_data = (int)data[0];

	// 表示の更新
	decode_left2right(hex, data);

	// 次のマトリクスに送るデータ
	xprintf("MATLED PUSH %d\n", flow_data);	
}

void set_anim(const char *input_buf , char *data , struct Frame *fr) {
	char* pbuf = (char *)input_buf + 11;
	int nf = parseInt(pbuf); 		// フレーム番号取得
	if (nf >= 0 && nf <= N_FRAME) {
		int n = indexOf(pbuf, ' ');
		if (n >= 0) {
			pbuf += n + 1;			// 表示データ取得
			decode_top2bottom(pbuf, fr[nf].frame);
			decode_top2bottom(pbuf, data); 	// 停止時の画面にも表示
			n = indexOf(pbuf, ' ');
			int nw = 100;
			if (n >= 0) {
				pbuf += n + 1;
				nw = parseInt(pbuf);
			}
			fr[nf].waitms = nw;
		}
	}
}

void app_uart() {
#define SIZE_BUF 128
	char	input_buf[SIZE_BUF];	// uartからのコマンド待ち状態
	int		nbuf = 0;		// uartからの入力文字数
	int		mode = 0;		// 0: 停止状態 1: RUN状態
	int		cnt = 0;		// RUN時のwaitカウンタ
	int		nframe = 0;		// 実行中のフレーム番号
	char	data[8];		// 表示バッファ

	static struct Save_data sd;
	struct Frame *fr = sd.frame;

	for (int i = 0; i < SIZE_BUF; i++)	input_buf[i] = 0;
	for (int i = 0; i < 8; i++)			data[i] = 0;

	CLS(1);
	
	for (int i = 0;; i++) {
		// deepSleepしない
		no_sleep();
		while (uart0_test()) {
			int c = uart0_getc();
			if (c == '\n') {
				input_buf[nbuf] = '\0';
				if (startsWith(input_buf, "MATLED SHOW ")) {
					mode = 0;
					show(input_buf , data);
				} else if (startsWith(input_buf, "MATLED PUSH ")) {
					mode = 0;
					push(input_buf , data);
				} else if (startsWith(input_buf, "MATLED SET ")) {
					mode = 0;
					set_anim(input_buf , data , fr);
				} else if (startsWith(input_buf, "MATLED CLEAR")) {
					mode = 0;
					init_frame(&sd);
					println("CLEAR");
				} else if (startsWith(input_buf, "MATLED RUN")) {
					mode = 1;
					println("RUN");
				} else if (startsWith(input_buf, "MATLED STOP")) {
					mode = 0;
					println("STOP");
				} else if (startsWith(input_buf, "MATLED SAVE")) {
					mode = 0;
					save(&sd);
					println("SAVE");
				} else if (startsWith(input_buf, "MATLED LOAD")) {
					mode = 0;
					load(&sd);
					println("LOAD");
				} else if (startsWith(input_buf, "MATLED BREAK")) {
					println("BREAK");
					return;
				} else {	// 不明なコマンドは、後続に接続されている器械にパススルーする。
					println(input_buf);
				}
				nbuf = 0;
				continue;
			} else if (c == '\r') {
			} else {
				if (nbuf < SIZE_BUF - 1) input_buf[nbuf++] = c;
			}
		}
		if (mode == 0) {
			set_matrix(data);
		} else {
			set_matrix(fr[nframe].frame);
			WAIT(1);
			cnt++;
			if (cnt >= fr[nframe].waitms) { // 待ち時間を過ぎたら
				cnt = 0;

				nframe++;
				if (nframe == N_FRAME || fr[nframe].waitms == 0) nframe = 0;	// ループして最初から
			}
		}
	}
}
