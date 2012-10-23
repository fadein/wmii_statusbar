CFLAGS=-std=gnu99 -ggdb -Wall -Wextra -pedantic

WMII_LDFLAGS=-lasound -lixp $(LDFLAGS)
DWM_LDFLAGS=-lasound -lX11 $(LDFLAGS)
CC=gcc

.PHONY: clean all tags

all: wmii_statusbar dwm_statusbar

objs = alsavolume.o proc.o
alsavolume.o: Makefile
proc.o: Makefile wmii_statusbar.h
wmii_statusbar.o: Makefile wmii_statusbar.h proc.h alsavolume.h
dwm_statusbar.o: Makefile dwm_statusbar.h proc.h alsavolume.h

wmii_statusbar: wmii_statusbar.c $(objs)
	gcc -o $@ $^ $(CFLAGS) $(WMII_LDFLAGS)

dwm_statusbar: dwm_statusbar.c $(objs)
	gcc -o $@ $^ $(CFLAGS) $(DWM_LDFLAGS)

clean:
	rm -f *.o a.out tags wmii_statusbar dwm_statusbar

tags:
	ctags *

simple_volume.o: simple_volume.c
	gcc -c -o $@ $^ $(CFLAGS)

test: wmii_statusbar
	./wmii_statusbar
