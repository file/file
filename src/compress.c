/*
 * compress routines:
 *	is_compress() returns 0 if uncompressed, number of bits if compressed.
 *	uncompress(old, n, newch) - uncompress old into new, return sizeof new
 */

/* Check for compression, return nbits. Algorithm, in magic(4) format:
 * 0       string          \037\235        compressed data
 * >2      byte&0x80       >0              block compressed
 * >2      byte&0x1f       x               %d bits
 */
int
is_compress(p, b)
unsigned char *p;
int *b;
{

	if (*p != '\037' || *(/*signed*/ char*)(p+1) != '\235')
		return 0;	/* not compress()ed */

	*b = *(p+2) & 0x80;
	return *(p+2) & 0x1f;
}

int
uncompress(old, n, newch)
unsigned char *old, **newch;
int n;
{
	*newch = old;	/* TODO write this */
	**newch = 0;	/* prevent infinite loop, skeleton version only */
	return n;
}
	
