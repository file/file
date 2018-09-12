/*
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
/*
 * LLVM fuzzing integration.
 */

#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: magic_fuzzer.c,v 1.1 2017/04/24 19:41:34 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <libgen.h>
#include <stdlib.h>
#include <err.h>

int LLVMFuzzerInitialize(int *, char ***);
int LLVMFuzzerTestOneInput(const uint8_t *, size_t);

static magic_t magic;

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
	char dfile[MAXPATHLEN], mfile[MAXPATHLEN];

	magic = magic_open(MAGIC_NONE);
	if (magic == NULL) {
		warn("magic_open");
		return -1;
	}

	// Poor man's strlcpy(3), to avoid potentially destructive dirname(3)
	snprintf(dfile, sizeof(dfile), "%s", (*argv)[0]);
	snprintf(mfile, sizeof(mfile), "%s/magic", dirname(dfile));

	if (magic_load(magic, mfile) == -1) {
		warnx("magic_load: %s", magic_error(magic));
		return -1;
	}

	return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size == 0)
		return 0;

	magic_buffer(magic, data, size);
	return 0;
}
