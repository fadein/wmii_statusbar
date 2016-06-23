#include <stdio.h>
#include <time.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <signal.h>

#include "proc.h"

#ifdef XKB
#include "xkb.h"
#endif

#ifdef ALSA
#include "alsavolume.h"
#endif

#ifdef FIFO
#include "fifo.h"
#endif

#include "statusbar.h"
#include "dwm_statusbar.h"

static Display *dpy;
static int batteryPercent = 0;
static int loadAve = 0;
static int timerMode = 0;
static int timerSecs = 0;
static time_t when = 0;
static int paused = 1;
static int timer_duration = 15;

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
		timerSecs = timer_duration * 60;

	else if (signum == SIGURG)
		// pause the timer
		paused ^= 1;

	else if (signum == SIGPOLL)
		// SIGPOLL adds one minute to the timer
		timerSecs += 61;
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
#ifdef DEBUG
	puts(str);
#else
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
#endif
}

#ifdef FIFO
static void cmdParse(char* cmds) {
	char *c = strtok(cmds, " \n\t");
	do {
		if      (!strcasecmp(c, "battery"))
			batteryPercent ^= 1;
		else if (!strcasecmp(c, "loadave"))
			loadAve ^= 1;
		else if (!strcasecmp(c, "timer"))
			timerMode ^= 1;
		else if (!strcasecmp(c, "reset"))
			timerSecs = timer_duration * 60;
		else if (!strcasecmp(c, "pause"))
			paused ^= 1;
		else if (!strcasecmp(c, "resume"))
			paused = 0;
		else if (*c == '+')
			timerSecs += (60 * atoi((const char*)++c)) + 1;
		else if (*c == '-')
			timerSecs -= (60 * atoi((const char*)++c)) + 1;
		else if (*c == '=') {
			timer_duration = atoi((const char*)++c);
			timerSecs = timer_duration * 60 + 1;
		}
	} while (NULL != (c = strtok(NULL, " \n\t")));
}
#endif

int
main(void)
{
	char out[SBAR];
	char *buf; 

#ifdef FIFO
	char *fifoCmd;
	fifoInit();
#endif

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "XOpenDisplay() FAIL\n");
		return 1;
	}

#define INSTALLSIGNAL(S, F) { if (SIG_ERR == signal(S, F)) \
	error_at_line(1, errno, __FILE__, __LINE__, "signal(" #S ") FAIL"); }

	INSTALLSIGNAL(SIGUSR1,  togglePresentation);
	INSTALLSIGNAL(SIGUSR2,  togglePresentation);
	INSTALLSIGNAL(SIGHUP,   togglePresentation);
	INSTALLSIGNAL(SIGQUIT,  togglePresentation);
	INSTALLSIGNAL(SIGWINCH, togglePresentation);
	INSTALLSIGNAL(SIGURG,   togglePresentation);
	INSTALLSIGNAL(SIGPOLL,  togglePresentation);

	if (NULL == (buf = (char*)malloc(sizeof(char) * READBUFFER)))
		error_at_line(1, errno, __FILE__, __LINE__, "malloc() FAIL");

	memset(&out, '\0', (size_t)SBAR);

	for (;; *out = '\0', sleep(SLEEPYTIME) ) {

#ifdef FIFO
		fifoCmd = fifoCheck();
		if (*fifoCmd)
			cmdParse(fifoCmd);
#endif

		if (!paused && timerSecs > 0)
			timerSecs--;

		if (timerMode) {
			snprintf(out, (size_t)SBAR,
					paused
					? "(%02d:%02d)"
					: "%02d:%02d",
					timerSecs / 60, timerSecs % 60);
		}
		else {
			/*
			 * get wifi state, if wifi is on - icons?
			 * which font had the hearts and stuff?
			 */
			//strncat(out, getWifi(buf), (size_t)SBAR);
			//strncat(out, " | ", (size_t) SBAR);

#ifdef ALSA
			strncat(out, "Vol:", (size_t)SBAR);
			strncat(out, getAlsaVolume("Master"), (size_t)SBAR);
			strncat(out, " ", (size_t) SBAR);
#endif

#ifdef XKB
			strncat(out, "Kb:", (size_t)SBAR);
			strncat(out, xkbGetGroup(buf, (size_t) 2), (size_t)SBAR);
			strncat(out, " ", (size_t) SBAR);
#endif

			strncat(out, getCPU(buf), (size_t)SBAR);
			strncat(out, " ", (size_t) SBAR);

			strncat(out, getMemory(buf), (size_t) SBAR);
			strncat(out, " ", (size_t) SBAR);

#ifdef BATTERY
			if (batteryPercent == 1)
				strncat(out, getBatteryPercent(buf), (size_t) SBAR);
			else
				strncat(out, getBatteryTime(buf), (size_t) SBAR);
			strncat(out, " ", (size_t) SBAR);
#endif

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
