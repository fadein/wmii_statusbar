#ifdef FIFO

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fifo.h"

static void mkfifoUnlessExists(void) {
	struct stat fifostat;
	if (stat(FIFO_PATH, &fifostat) < 0) {
		if (mkfifo(FIFO_PATH, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP)) {
			perror("mkfifo");
			exit(EXIT_FAILURE);
		}
	}
	else if (!S_ISFIFO(fifostat.st_mode)) {
		fprintf(stderr, FIFO_PATH " already exists, but is not a FIFO!\n");
		exit(EXIT_FAILURE);
	}
}

//open the fifo to an FD
int fifo, n;

// add that FD to the FD_SET
fd_set readFD;
struct timeval timeout;
char buf[FIFO_BUFSZ];
unsigned i = 0; // DELETE ME

int fifoInit() {
	mkfifoUnlessExists();
	fifo = open(FIFO_PATH, O_RDWR | O_NONBLOCK);
	return fifo;
}

int fifoFree(void) {
	return close(fifo);
}

void fifoCheck(void) {
		printf("tick %d\n", i); // DELETE ME
		FD_SET(fifo, &readFD);
		timeout.tv_sec = 0;
		timeout.tv_usec = 1;
		n = select(fifo + 1, &readFD, NULL, NULL, &timeout);
		if (n < 0) {
			perror("select");
			return;
		}
		else if (n) {
			printf("n = %d\n", n); //DELETE ME
			if (FD_ISSET(fifo, &readFD)) {
				ssize_t bytes = 0;
				size_t totes = 0;
				while (1) {
					bytes = read(fifo, buf, sizeof(buf));
					if (bytes > 0) {
						totes += bytes;
						printf("!read %zd bytes into buf\n", bytes); // DELETE ME
					}
					else {
						if (errno == EWOULDBLOCK) {
							printf("@read %zd bytes in total\n", totes); // DELETE ME
							if (totes < sizeof(buf))
								buf[totes] = '\0';
							else
								buf[sizeof(buf)] = '\0';
							printf("%s\n", buf); // DELETE ME
							break;
						}
						else {
							perror("read");
							return;
						}
					}
				}
			}
		}
		printf("tock %d\n", i++); // DELETE ME
}

#endif
