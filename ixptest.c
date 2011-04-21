/* gcc ixptest.c -o ixptest -lixp */

#include <ixp.h>
#include <string.h>

#define fatal(...) ixp_eprint("ixpc: fatal: " __VA_ARGS__); \

static IxpClient *client;

int main(void) {
	IxpCFid *fid;
	int len;
	char *message = "This test is a test";
	char *fname = "/rbar/status";

	len = strlen(message);

	client = ixp_nsmount("wmii");
	if (NULL == client) {
		fatal("can't mount: %r\n");
	}

	fid = ixp_open(client, fname, P9_OWRITE);

	if (len != ixp_write(fid, message, len)) {
		fatal("cannot write file %q\n", fname);
	}

	ixp_close(fid);
	ixp_unmount(client);
	return 0;
}
