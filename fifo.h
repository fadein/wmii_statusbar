#ifndef FIFO_H

#define FIFO_PATH "/tmp/dwm_statusbar"
#define FIFO_BUFSZ 80


int fifoInit(void);
void fifoCheck(void);
int fifoFree(void);


#define FIFO_H
#endif
