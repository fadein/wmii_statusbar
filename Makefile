# Valid features are:
# BATTERY - read battery stats from /proc/acpi/battery/BAT0/state
# ALSA    - query ALSA for sound card volume
# XKB     - query XKB for current X11 keyboard layout
# FIFO    - allow for control through fifo at /tmp/dwm_statusbar
FEATURES=BATTERY ALSA XKB FIFO

CFLAGS=-std=gnu99 -ggdb -Wall -Wextra -pedantic $(addprefix -D,$(FEATURES))

WMII_LDFLAGS=-lasound -lm -lixp $(LDFLAGS)
DWM_LDFLAGS=-lasound -lm -lX11 $(LDFLAGS)
CC=gcc
CSC=csc
CSC_FLAGS=-strip -O5

.PHONY: clean all tags

all: dwm_statusbar

objs = proc.o
$(if $(findstring ALSA,$(FEATURES)),$(eval objs += alsavolume.o))
$(if $(findstring XKB,$(FEATURES)),$(eval objs += xkb.o))
$(if $(findstring FIFO,$(FEATURES)),$(eval objs += fifo.o))

wmii_statusbar.o: Makefile wmii_statusbar.h proc.h alsavolume.h
dwm_statusbar.o: Makefile dwm_statusbar.h proc.h alsavolume.h xkb.h statusbar.h fifo.h


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
