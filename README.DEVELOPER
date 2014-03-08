# How to get started developing

@(#) $File: README.DEVELOPER,v 1.2 2014/03/06 16:57:57 christos Exp $

## Auto files

After checking out the source, run the following:

	autoreconf -f -i
	./configure --disable-silent-rules
	make -j4
	make -C tests check


## Installing dependencies

If your platform doesn't have the above tools, install the following
packages first.

### Debian

	apt-get install \
	    automake \
	    autotools-dev \
	    gcc \
	    make \
	    libtool \
	    python \
	    zlib1g-dev \

See also `.travis.yml`.

### Mac OS X (MacPorts)

	port install \
	    autoconf \
	    automake \
	    libtool \

