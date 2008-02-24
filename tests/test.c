/*
 * Copyright (c) Christos Zoulas 2003.
 * All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stdio.h>
#include "magic.h"

int
main(int argc, char **argv)
{
	struct magic_set *ms;
	const char *m;
	int i;

	ms = magic_open(MAGIC_NONE);
	if (ms == NULL) {
		fprintf(stderr, "ERROR opening MAGIC_NONE: out of memory\n");
		return 1;
	}
	if (magic_load(ms, NULL) == -1) {
		fprintf(stderr, "ERROR loading with NULL file: %s\n", magic_error(ms));
		return 2;
	}

	if (argc > 1) {
		if (argc != 3) {
			fprintf(stderr, "Usage: test TEST-FILE RESULT\n");
		} else {
			if ((m = magic_file(ms, argv[1])) == NULL) {
				fprintf(stderr, "ERROR loading file %s: %s\n", argv[1], magic_error(ms));
				return 3;
			} else {
				printf("%s: %s\n", argv[1], m);
					/* Compare m with contents of argv[3] */
			}
		}
	}

	magic_close(ms);
	return 0;
}
