#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "proc.h"

#define BUFSIZE 512

int
main (void)
{
	char out[BUFSIZE];

	memset(&out, '\0', BUFSIZE);

	printf("%s\n", getBatteryTime((char *)&out));

	return 0;
}
