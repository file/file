/*
 * Ascii magic -- file types that we know based on keywords
 * that can appear anywhere in the file.
 */

#include <stdio.h>
#include <ctype.h>
#include "file.h"
char ckfmsg[] = "write error on output";
#include "names.h"

			/* an optimisation over plain strcmp() */
#define	STREQ(a, b)	(*(a) == *(b) && strcmp((a), (b)) == 0)

ascmagic(buf)
register char	*buf;
{
	register int i;
	char	*s, *strtok(), *token;
	register struct names *p;
	extern int nbytes;
	short has_escapes = 0;

	/* these are easy, do them first */

	/*
	 * for troff, look for . + letter + letter;
	 * this must be done to disambiguate tar archives' ./file
	 * and other trash from real troff input.
	 */
	if (*buf == '.' && 
		isascii(*(buf+1)) && isalnum(*(buf+1)) &&
		isascii(*(buf+2)) && isalnum(*(buf+2))){
		ckfputs("troff or preprocessor input text", stdout);
		return 1;
	}
	if ((*buf == 'c' || *buf == 'C') && 
	    isascii(*(buf + 1)) && isspace(*(buf + 1))) {
		ckfputs("fortran program text", stdout);
		return 1;
	}

	/* look for tokens from names.h - this is expensive! */
	s = buf;
	while ((token = strtok(s, " \t\n\r\f")) != NULL) {
		s = NULL;	/* make strtok() keep on tokin' */
		for (p = names; p < names + NNAMES; p++) {
			if (STREQ(p->name, token)) {
				ckfputs(types[p->type], stdout);
				return 1;
			}
		}
	}

	if (is_tar(buf)) {
		ckfputs("tar archive", stdout);
		return 1;
	}

	for (i = 0; i < nbytes; i++) {
		if (!isascii(*(buf+i)))
			return 0;	/* not all ascii */
		if (*(buf+i) == '\033')	/* ascii ESCAPE */
			has_escapes ++;
	}

	/* all else fails, but it is ascii... */
	if (has_escapes){
		ckfputs("ascii text (with escape sequences)", stdout);
		}
	else {
		ckfputs("ascii text", stdout);
		}
	return 1;
}


