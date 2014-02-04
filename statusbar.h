#ifndef STATUSBAR_H
#define STATUSBAR_H

/*
 * Max number of bytes to read from a /proc file
 */
#define READBUFFER 4096

/*
 * format string for strftime()
 */
#define TIME_FMT "%a %D %I:%M%p"


/*
 * how many characters wide the statusbar shall be
 */
#define SBAR 64

/*
 * How many seconds to sleep between updates
 */
#define SLEEPYTIME 1

#endif
