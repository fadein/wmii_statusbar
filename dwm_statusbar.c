#include <stdio.h>
#include <time.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <signal.h>

#include "proc.h"
#include "xkb.h"
#include "alsavolume.h"
#include "statusbar.h"
#include "dwm_statusbar.h"

static Display *dpy;
static int batteryPercent = 0;
static int loadAve = 0;

/*
 * toggle the presentation of certain datas upon receipt of signals
 */
void togglePresentation(int signum) {
	if (signum == SIGUSR1)
		batteryPercent ^= 1;
	else if (signum == SIGUSR2)
		loadAve ^= 1;
}

/*
 * get date & time
 */
char* getDateTime(char* buf) {
	static time_t curtime;
	static struct tm *loctime;

	curtime = time(NULL);
	loctime = localtime(&curtime);
	strftime(buf, SBAR, TIME_FMT, loctime);
	return buf;
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

int
main(void)
{
	char out[SBAR];
	char *buf;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "XOpenDisplay() FAIL\n");
		return 1;
	}

	if (SIG_ERR == signal(SIGUSR1, togglePresentation))
		error_at_line(1, errno, __FILE__, __LINE__, "signal(SIGUSR1) FAIL");

	if (SIG_ERR == signal(SIGUSR2, togglePresentation))
		error_at_line(1, errno, __FILE__, __LINE__, "signal(SIGUSR1) FAIL");

	if (NULL == (buf = (char*)malloc(sizeof(char) * READBUFFER)))
		error_at_line(1, errno, __FILE__, __LINE__, "malloc() FAIL");

	memset(&out, '\0', (size_t)SBAR);

	for (;; *out = '\0', sleep(SLEEPYTIME) ) {

		/*
		 * get wifi state, if wifi is on - icons?
		 * which font had the hearts and stuff?
		 */
		//strncat(out, getWifi(buf), (size_t)SBAR);
		//strncat(out, " | ", (size_t) SBAR);

		strncat(out, "Vol:", (size_t)SBAR);
		strncat(out, getAlsaVolume("Master"), (size_t)SBAR);
		strncat(out, " ", (size_t) SBAR);

		strncat(out, "Kb:", (size_t)SBAR);
		strncat(out, xkbGetGroup(buf, (size_t) 2), (size_t)SBAR);
		strncat(out, " ", (size_t) SBAR);

		strncat(out, getCPU(buf), (size_t)SBAR);
		strncat(out, " ", (size_t) SBAR);

		strncat(out, getMemory(buf), (size_t) SBAR);
		strncat(out, " ", (size_t) SBAR);

		if (batteryPercent == 1)
			strncat(out, getBatteryPercent(buf), (size_t) SBAR);
		else
			strncat(out, getBatteryTime(buf), (size_t) SBAR);
		strncat(out, " ", (size_t) SBAR);


		if (loadAve == 1) {
			strncat(out, "|", (size_t) SBAR);
			strncat(out, getLoadAve(buf), (size_t) SBAR);
			strncat(out, "|", (size_t) SBAR);
		}
		else
			strncat(out, getDateTime(buf), (size_t) SBAR);

		setstatus(out);
	}
}
