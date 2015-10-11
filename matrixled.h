/*
matrixled
*/
#ifndef _MATRIXLED
#define _MATRIXLED

// フレームの全体長
#define LEN_DATA 1024

// フレーム総数
#define N_FRAME ((LEN_DATA - 8) / 10)
//                           ^    ^ struct Frameの大きさ
//                           | save_tagの大きさ

// 1画面分のデータ
struct Frame {
	char frame[8];
	short waitms;
};

// Saveデータ全体 識別フラグ付き
struct Save_data {
	char header[8];
	struct Frame frame[N_FRAME];
};

void matrixled_init();
void matrixled_on(int x, int y);
void matrixled_off();
void matrixled_tick();

void get_matrix(char* data);
void set_matrix(const char* data);
void put_matrix(const char* s, int dx, int dy, int dw, int dh);
void PSET(int x, int y);
void PRESET(int x, int y);
void CLS(int n);
void FILL(const char *hex_string);
void FLUSH();

extern short g_rotate;

#endif
