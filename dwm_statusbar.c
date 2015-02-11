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
static int timerMode = 0;
static int timerSecs = 0;
static time_t when = 0;

/*
 * toggle the presentation of certain datas upon receipt of signals
 * Note: the signal handler interrupts sleep - and so will steal up to a second from the timer!
 */
void togglePresentation(int signum) {
	if (signum == SIGUSR1)
		batteryPercent ^= 1;

	else if (signum == SIGUSR2)
		loadAve ^= 1;

	else if (signum == SIGHUP)
		// display the countdown timer when MOD4+colon are pressed
		timerMode ^= 1;

	else if (signum == SIGWINCH)
		// if the backtick is signaled, note the time...
		when = time(NULL);

	else if (signum == SIGQUIT && time(NULL) - when < 2)
		// only reset the time if this signal happens within a second of SIGWINCH
		timerSecs = 50 * 60;
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

#define INSTALLSIGNAL(S, F) { if (SIG_ERR == signal(S, F)) \
	error_at_line(1, errno, __FILE__, __LINE__, "signal(" #S ") FAIL"); }

	INSTALLSIGNAL(SIGUSR1, togglePresentation);
	INSTALLSIGNAL(SIGUSR2, togglePresentation);
	INSTALLSIGNAL(SIGHUP, togglePresentation);
	INSTALLSIGNAL(SIGQUIT, togglePresentation);
	INSTALLSIGNAL(SIGWINCH, togglePresentation);

	if (NULL == (buf = (char*)malloc(sizeof(char) * READBUFFER)))
		error_at_line(1, errno, __FILE__, __LINE__, "malloc() FAIL");

	memset(&out, '\0', (size_t)SBAR);

	for (;; *out = '\0', sleep(SLEEPYTIME) ) {

		if (timerSecs > 0)
			timerSecs--;

		if (timerMode) {
			snprintf(out, (size_t)SBAR, "%02d:%02d", timerSecs / 60, timerSecs % 60);
		}
		else {
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
		}

		setstatus(out);
	}
}
