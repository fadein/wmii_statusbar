#include <stdio.h>
#include <time.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "proc.h"
#include "xkb.h"
#include "alsavolume.h"
#include "statusbar.h"
#include "dwm_statusbar.h"

static Display *dpy;

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

		strncat(out, getBatteryTime(buf), (size_t) SBAR);
		strncat(out, " ", (size_t) SBAR);

		/* strncat(out, getLoadAve(buf), (size_t) SBAR); */
		/* strncat(out, " ", (size_t) SBAR); */

		strncat(out, getDateTime(buf), (size_t) SBAR);
		/* strncat(out, " | ", (size_t) SBAR);*/

		/*
		printf("\n\nthe string:\n");
		printf("%s", out);
		printf("\n");
		*/

		setstatus(out);
	}
}
