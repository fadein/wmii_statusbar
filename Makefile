CFLAGS=-std=gnu99 -ggdb -Wall -Wextra -pedantic
LDFLAGS=-lasound -lixp 
CC=i686-pc-linux-gnu-gcc

.PHONY: clean all tags

all: wmii_statusbar

objs = wmii_statusbar.o alsavolume.o proc.o
alsavolume.o: Makefile
proc.o: Makefile wmii_statusbar.h
wmii_statusbar.o: Makefile wmii_statusbar.h proc.h alsavolume.h

wmii_statusbar: $(objs)
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o a.out tags wmii_statusbar

tags:
	@echo Plz install exuberant-ctags, yo

simple_volume.o: simple_volume.c
	gcc -c -o $@ $^ $(CFLAGS)

test: wmii_statusbar
	./wmii_statusbar
