#ifndef _MAGIC_H
#define _MAGIC_H

#include <sys/types.h>

#define	MAGIC_NONE	0x00	/* No flags */
#define	MAGIC_DEBUG	0x01	/* Turn on debugging */
#define	MAGIC_SYMLINK	0x02	/* Follow symlinks */
#define	MAGIC_COMPRESS	0x04	/* Check inside compressed files */
#define	MAGIC_DEVICES	0x08	/* Look at the contents of devices */
#define	MAGIC_MIME	0x10	/* Return a mime string */
#define	MAGIC_CONTINUE	0x20	/* Return all matches, not just the first */
#define	MAGIC_CHECK	0x40	/* Print warnings to stderr */

struct magic_set;

struct magic_set *magic_open(int flags);
void magic_close(struct magic_set *);

const char *magic_file(struct magic_set *, const char *);
const char *magic_buf(struct magic_set *, const void *, size_t);

const char *magic_error(struct magic_set *);
void magic_setflags(struct magic_set *, int);

int magic_load(struct magic_set *, const char *);
int magic_compile(struct magic_set *, const char *);
int magic_check(struct magic_set *, const char *);

#endif
