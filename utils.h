/*
IchigoDot Utilitys Header
*/
#ifndef _UTIL
#define _UTIL

#include "xprintf.h"

#define boolean unsigned char
#define true 1
#define false 0

boolean startsWith(const char* target, const char* key);
int  parseInt(const char* target);
int  indexOf(const char* target, char search_char);
void println(const char* string);
int  decode_left2right(const char* src, char* dst);
void decode_top2bottom(const char* src, char* dst);
unsigned int rnd();

#endif
