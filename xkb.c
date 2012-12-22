#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xlibint.h>
#include <X11/Xatom.h>
#include "statusbar.h"

char* xkbGetGroup(char* buf, int len) {
	int  xkbEventType, xkbError, reason_rtrn, mjr, mnr;
	char *display_name;
	XkbStateRec xkbstate;

	static Display *dpy = NULL;
	static XkbDescPtr xkbdesc = NULL;

	/* Lets begin */
	if (!dpy) {
		display_name = NULL;
		mjr = XkbMajorVersion;
		mnr = XkbMinorVersion;
		dpy = XkbOpenDisplay(display_name, &xkbEventType, &xkbError,
				&mjr, &mnr, &reason_rtrn);
	}

	if (dpy == NULL) {
		strcpy(buf, "?");
		return buf;
	}

	if ( Success != XkbGetState(dpy, XkbUseCoreKbd, &xkbstate) ) {
		strcpy(buf, "?");
		return buf;
	}

	if (!xkbdesc)
		xkbdesc = XkbAllocKeyboard();

	if (!xkbdesc) {
		strcpy(buf, "?");
		return buf;
	}

	/* get the names of the layout */
	if ( Success == XkbGetNames(dpy, 1<<12, xkbdesc)) {
		Atom iatoms[4];
		char *iatomnames[4];
		int i, j;

		for (i = 0, j = 0; i < 4; i++)
			if (xkbdesc->names->groups[i] != None)
				iatoms[j++] = xkbdesc->names->groups[i];

		if (XGetAtomNames(dpy, iatoms, j, iatomnames))
			buf = strndup(iatomnames[xkbstate.locked_group], len);
	}

	return buf;
}
