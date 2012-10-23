#include <stdio.h>
#include <time.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "statusbar.h"

/*
 * Read READBUFFER bytes from the specified /proc file
 * into *uf, 
 * also return pointer to buf
 */
static char* readProcFile(const char* file, char* buf) {
	static size_t count;
	static FILE *info;
	static char procpath[PATH_MAX];

	strcpy((char*)&procpath, "/proc/");
	strcat((char*)&procpath, file);

	if (NULL == (info = fopen((char*)&procpath, "r"))) {
		error_at_line(1, errno, __FILE__, __LINE__, "fopen(%s) FAIL", file);
	}

	count = fread(buf, sizeof(char), READBUFFER, info);
	if (ferror(info)) {
		error_at_line(1, errno, __FILE__, __LINE__, "ferror(%s) FAIL", file);
	}

	if (-1 == fclose(info)) {
		error_at_line(1, errno, __FILE__, __LINE__, "fclose(%s) FAIL", file);
	}

	return buf;
}


/*
 * get load average
 */
char* getLoadAve(char* buf) {
	char* c = readProcFile("loadavg", buf);
	for (int i = 0; i <= 2; i++) {
		c = strchr(c, ' ') + 1;
	}
	*c = '\0';
	return buf;
}

/*
 * get CPU%
 *
 * in a loop:
 *  read /proc/stat for line beginning with "cpu"
 *  save the first four columns of numbers: they correspond to
 *  ticks spent in:
 *       user   nice system idle
 *  cpu  300745 9636 211625 3366592 44847 57 1811 0 0 0
 *
 *  take the difference between the fresh values & last times'
 *  apply this formula:
 *  %cpu = (user+system)/(user+system+nice+idle)
 */
char* getCPU(char* buf) {
	static unsigned int user = 0,
						nice = 0,
						system = 0,
						idle = 0,
						prvUser = 0,
						prvNice = 0,
						prvSystem = 0,
						prvIdle = 0,
						difUser = 0,
						difNice = 0,
						difSystem = 0,
						difIdle = 0;

	readProcFile("stat", buf);
	strtok(buf, " ");                 /* throw away cpu label */

	if (0 == prvUser) {
		prvUser = atoi(strtok(NULL, " "));   /* get user ticks */
		prvNice = atoi(strtok(NULL, " "));   /* get nice ticks */
		prvSystem = atoi(strtok(NULL, " ")); /* get system ticks */
		prvIdle = atoi(strtok(NULL, " "));   /* get idle ticks */
		sprintf(buf, "CPU:N/A");
	}
	else {
		user = atoi(strtok(NULL, " "));   /* get user ticks */
		nice = atoi(strtok(NULL, " "));   /* get nice ticks */
		system = atoi(strtok(NULL, " ")); /* get system ticks */
		idle = atoi(strtok(NULL, " "));   /* get idle ticks */

		difUser = user - prvUser;
		difNice = nice - prvNice;
		difSystem = system - prvSystem;
		difIdle = idle - prvIdle;

		prvUser = user;
		prvNice = nice;
		prvSystem = system;
		prvIdle = idle;

		sprintf(buf, "CPU:%d%%",
					100 * (difUser + difSystem)
								   /
				(difUser + difSystem + difNice + difIdle));
	}
	return buf;
}

/*
 * get memory state - swap, etc
 * read lines from /proc/meminfo:
	MemTotal:        2054004 kB
	MemFree:         1158208 kB
	Buffers:          109108 kB
	Cached:           368460 kB
	SwapCached:         5452 kB
	Active:           440056 kB
	Inactive:         315384 kB
	Active(anon):     235768 kB
	Inactive(anon):    42564 kB
	Active(file):     204288 kB
	Inactive(file):   272820 kB
	Unevictable:         132 kB
	Mlocked:             132 kB
	SwapTotal:      32017504 kB
	SwapFree:       31989536 kB
	Dirty:                64 kB
	Writeback:             0 kB
	AnonPages:        273248 kB
	Mapped:            62188 kB
	Shmem:               460 kB
	Slab:              88472 kB
	SReclaimable:      70688 kB
	SUnreclaim:        17784 kB
	KernelStack:        2016 kB
	PageTables:         8724 kB
	NFS_Unstable:          0 kB
	Bounce:                0 kB
	WritebackTmp:          0 kB
	CommitLimit:    33044504 kB
	Committed_AS:     796424 kB
	VmallocTotal:   34359738367 kB
	VmallocUsed:      117968 kB
	VmallocChunk:   34359570940 kB
	DirectMap4k:      777152 kB
	DirectMap2M:     1318912 kB
 * Take the ratio of MemFree / MemTotal
 */
char* getMemory(char* buf) {
	static int tot,
			   free;

	readProcFile("meminfo", buf);
	strtok(buf, " ");
	tot = atoi(strtok(NULL, " ")); /* get the total memory */
	strtok(NULL, " "); /* skip kB, \n, MemFree: */
	free = atoi(strtok(NULL, " ")); /* get free memory */
	sprintf(buf, "Mem:%d%%", 100 - free * 100/tot);
	return buf;
}


/*
 * get battery time left
 */
char* getBatteryTime(char* buf) {
	char *present, *discharging;
	int rate, remaining;
	int hours, minutes;

	readProcFile("acpi/battery/BAT0/state", buf);

	/* printf("read in:\n===\n%s\n===\n\n", buf); */ /*DELETE ME*/

	/* is battery present? */
	strtok(buf, " ");
	present = strtok(NULL, " ");
	/* printf("PRESENT: %s\n", present); */ /* DELETE ME */
	/* printf("PRESENT? %c\n\n", *present == 'y' ? 'y' : 'n'); */ /* DELETE ME */

	if (*present == 'y') {
		/* discharging? */
		strtok(NULL, " ");
		strtok(NULL, " ");
		strtok(NULL, " ");
		discharging = strtok(NULL, " ");
		/* printf("DISCHARGING:\n===\n%s\n===\n\n", discharging); */ /* DELETE ME */
		/* printf("DISCHARGING? %c\n\n", *discharging == 'd' ? 'y' : 'n'); */ /* DELETE ME */

		/* rate */
		strtok(NULL, " ");
		rate = atoi(strtok(NULL, " "));
		/* printf("RATE:\n===\n%d\n===\n\n", rate); */ /* DELETE ME */

		if (0 == rate) {
			snprintf(buf, SBAR, "Bat:FULL%c",
					*discharging == 'd' ? '-' : '+');
		}
		else {
			/* remaining capacity, converted to mA minutes */
			strtok(NULL, " ");
			strtok(NULL, " ");
			remaining = 60 * atoi(strtok(NULL, " "));
			/* printf("REMAINING:\n===\n%d\n===\n\n", remaining); */ /* DELETE ME */

			minutes = remaining / rate;
			hours = minutes / 60;
			minutes %= 60;

			snprintf(buf, SBAR, "Bat:%d.%02d%c",
					hours,
					minutes,
					*discharging == 'd' ? '-' : '+');
		}
	}
	else {
		snprintf(buf, SBAR, "ACpwr");
	}

	return buf;
}

