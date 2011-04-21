#include <stdio.h>
#include <time.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <ixp.h>

#include "proc.h"
#include "alsavolume.h"
#include "wmii_statusbar.h"


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

#define fatal(...) ixp_eprint("ixpc: fatal: " __VA_ARGS__);

static int ixpWrite(IxpClient *client, char* buf, int buflen) {
	static IxpCFid *fid;
	int len;

	fid = ixp_open(client, STATUSBAR_FILE, P9_OWRITE);

	if (buflen != (len = ixp_write(fid, buf, buflen))) {
		fatal("cannot write file %q\n", STATUSBAR_FILE);
	}

	ixp_close(fid);
	return len;
}

int main(void) {
	char out[SBAR];
	IxpClient *client;
	char *buf = (char*)malloc(sizeof(char) * READBUFFER);

	/* initiate connection to IxpClient */
	client = ixp_nsmount("wmii");
	if (NULL == client) {
		fatal("can't mount: %r\n");
	}

	memset(&out, '\0', (size_t)SBAR);

	if (NULL == buf) {
		error_at_line(1, errno, __FILE__, __LINE__, "malloc() FAIL");
	}

	for(;;) {

		*out = '\0';
		/*
		 * get battery state
		 * /proc/acpi/battery/BAT0/state
		 */
		//strncat(out, getBattery(buf), (size_t)SBAR);
		//strncat(out, " | ", (size_t) SBAR);

		/*
		 * get wifi state, if wifi is on - icons?
		 * which font had the hearts and stuff?
		 */
		//strncat(out, getWifi(buf), (size_t)SBAR);
		//strncat(out, " | ", (size_t) SBAR);

		/*
		 * get mixer volume
		 */
		strncat(out, "Vol:", (size_t)SBAR);
		strncat(out, getAlsaVolume("Master"), (size_t)SBAR);
		strncat(out, " ", (size_t) SBAR);

		strncat(out, getCPU(buf), (size_t)SBAR);
		strncat(out, " ", (size_t) SBAR);

		strncat(out, getMemory(buf), (size_t) SBAR);
		strncat(out, " ", (size_t) SBAR);

		strncat(out, getLoadAve(buf), (size_t) SBAR);
		/* strncat(out, " ", (size_t) SBAR); */

		strncat(out, getDateTime(buf), (size_t) SBAR);
		/* strncat(out, " | ", (size_t) SBAR);*/

		/* printf("\n\nthe string:\n"); */ /* DELETE ME */
		/* printf("%s", out); */
		/* printf("\n"); */ /* DELETE ME */

		ixpWrite(client, out, strlen(out));
		sleep(SLEEPYTIME);
	}

	ixp_unmount(client);
}
