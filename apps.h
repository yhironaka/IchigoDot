/*
apps.h
*/

#ifndef _APPS
#define _APPS

#include "utils.h"

boolean app_animate(const char* data, int len);
void app_uart();
void app_renda();
void app_hit10();
void app_mikuji();
void app_bitman();

#define PTN_3  "0038040438040438" // 3
#define PTN_2  "003804040810203c" // 2
#define PTN_1  "001828080808083c" // 1
#define PTN_GO "0073958585b59576" // GO

extern char* PTN_NUM[];

#endif _APPS
