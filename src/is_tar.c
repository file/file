/*
 * istar() - see if it is a tar file. Found in code written by
 *	Fred Blonder (301) 454-7690
 *	harpo!seismo!umcp-cs!fred
 *	Fred@Maryland.ARPA
 *	Net.sources, <9@gyre.uucp>, October 1983
 * I changed the calling sequence, added a cast or two, and ran through cb -sj.
 */

is_tar(buf)	/* Is it a tar file? */
char *buf;
{
	/* This stuff is copied from tar.c. */
#define TBLOCK	512
#define NAMSIZ	100
	register struct header {
		char	name[NAMSIZ];
		char	mode[8];
		char	uid[8];
		char	gid[8];
		char	size[12];
		char	mtime[12];
		char	chksum[8];
		char	linkflag;
		char	linkname[NAMSIZ];
	};
	register comp_chksum;
	register char	*cp;
	int	header_chksum;

	/* Compute checksum */
	comp_chksum = 0;
	for (cp = buf; cp < &buf[TBLOCK]; cp++)
		comp_chksum += (cp < ((struct header *)buf)->chksum
		     || cp >= &((struct header *)buf)->chksum[8]) ? 
		    *cp : ' ';

	/* Convert checksum field to integer */
	(void) sscanf(((struct header *)buf)->chksum, "%o", &header_chksum);

	return (comp_chksum == header_chksum); /* Is checksum correct? */
}


