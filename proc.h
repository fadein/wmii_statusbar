#ifndef PROC_H

char* getLoadAve(char* buf);
char* getCPU(char* buf);
char* getMemory(char* buf);

#ifdef BATTERY
char* getBatteryTime(char* buf);
char* getBatteryPercent(char* buf);
#endif

#define PROC_H
#endif
