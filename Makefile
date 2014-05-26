CFLAGS=-std=gnu99 -ggdb -Wall -Wextra -pedantic

WMII_LDFLAGS=-lasound -lm -lixp $(LDFLAGS)
DWM_LDFLAGS=-lasound -lm -lX11 $(LDFLAGS)
CC=gcc
CSC=csc
CSC_FLAGS=-strip -O5

.PHONY: clean all tags

all: dwm_statusbar

objs = alsavolume.o proc.o xkb.o
alsavolume.o: Makefile
proc.o: Makefile
xkb.o: Makefile
wmii_statusbar.o: Makefile wmii_statusbar.h proc.h alsavolume.h
dwm_statusbar.o: Makefile dwm_statusbar.h proc.h alsavolume.h xkb.h statusbar.h


wmii_statusbar: wmii_statusbar.c $(objs)
	$(CC) -o $@ $^ $(CFLAGS) $(WMII_LDFLAGS)

dwm_statusbar: dwm_statusbar.o $(objs)
	$(CC) -o $@ $^ $(CFLAGS) $(DWM_LDFLAGS)

battery: battery.c proc.o
	$(CC) -o $@ $^ $(CFLAGS)
	strip $@

chicken_battery: chicken_battery.scm proc.o
	$(CSC) $(CSC_FLAGS) -o $@ $^

clean:
	rm -f *.o a.out tags wmii_statusbar dwm_statusbar

tags:
	ctags *

simple_volume.o: simple_volume.c
	gcc -c -o $@ $^ $(CFLAGS)

test: wmii_statusbar
	./wmii_statusbar
