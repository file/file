#include <stdio.h>
#include "magic.h"

int
main(int argc, char **argv)
{
    struct magic_set *ms;
    const char *m;
    int i;

    if(argc < 2)
	return 1;

    ms = magic_open(MAGIC_NONE);
    if (ms == NULL) {
	printf("ERROR: out of memory\n");
	return 1;
    }
    if (magic_load(ms, NULL) == -1) {
	printf("ERROR: %s\n", magic_error(ms));
	return 1;
    }

    for (i = 1; i < argc; i++) {
	if ((m = magic_file(ms, argv[i])) == NULL)
	    printf("ERROR: %s\n", magic_error(ms));
	else
	    printf("%s: %s\n", argv[i], m);
    }

    magic_close(ms);
    return 0;
}
