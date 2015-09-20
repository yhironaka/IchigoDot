/*
matrixled
*/
#ifndef _MATRIXLED
#define _MATRIXLED

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

#endif
