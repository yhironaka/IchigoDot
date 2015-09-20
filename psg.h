/*
psg.h
*/
#ifndef _PSG
#define _PSG

void toggleSounder();
void CT16B0_IRQHandler();
void startPSG();
void stopPSG();
void psg_init();
void psg_tick();
void playMML(char* mml);

#endif
