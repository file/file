#!/bin/sh
# Cross-build file(1)/libmagic for m68k AmigaOS using the
# sacredbanana/amiga-compiler:m68k-amigaos Docker image.
#
# Builds against libnix (-noixemul), with file's own object files also
# compiled -fbaserel per project convention. libnix has no POSIX regex or
# working fork()/select(); see src/REGEX-COPYRIGHT and the AC_REPLACE_FUNCS
# entries in configure.ac for how those gaps are covered.
#
# Usage: contrib/amiga/build.sh [output-dir]
#
# Produces, in output-dir (default: ./out-amiga):
#   file        - m68k AmigaOS executable
#   libmagic.a  - static library, -noixemul -fbaserel object code
#   magic.h     - public header for libmagic.a
#   magic.mgc   - compiled magic database (architecture-independent;
#                 compiled here with a native host build of the same tree)

set -e

TOPDIR=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
OUT="${1:-$TOPDIR/out-amiga}"
mkdir -p "$OUT"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
IMAGE=sacredbanana/amiga-compiler:m68k-amigaos
BUILD_NATIVE="$TOPDIR/build-native"
BUILD_AMIGA="$TOPDIR/build-amiga"

mkdir -p "$BUILD_NATIVE" "$BUILD_AMIGA"

echo "==> regenerating autotools files (configure.ac/Makefile.am changed for this port)"
docker run --rm -v "$TOPDIR":/src -w /src "$IMAGE" bash -lc 'autoreconf -fi'

echo "==> native build (used only to compile the magic database, which is architecture-independent)"
# The image's plain "gcc"/"cc"/"ld"/"ar"/"ranlib" are all symlinked to the
# m68k-amigaos cross tools; the aarch64-linux-gnu-* ones (plus ld.bfd) are
# the real native compiler/binutils the container also ships.
docker run --rm \
	-v "$TOPDIR":/src \
	-v "$BUILD_NATIVE":/build-native \
	-w /build-native "$IMAGE" bash -lc '
		unset CROSS_PFX CROSS_ROOT CMAKE_TOOLCHAIN_FILE CMAKE_PREFIX_PATH
		CC="gcc-15 -fuse-ld=bfd" AR=aarch64-linux-gnu-ar RANLIB=aarch64-linux-gnu-ranlib LD=ld.bfd \
		/src/configure --disable-shared --enable-static --disable-dependency-tracking
		make -j"$(nproc)"
	'

echo "==> cross configure for m68k-amigaos (libnix / -noixemul)"
docker run --rm \
	-v "$TOPDIR":/src \
	-v "$BUILD_AMIGA":/build-amiga \
	-w /build-amiga "$IMAGE" bash -lc '
		export PATH=/opt/m68k-amigaos/bin:$PATH
		CC=m68k-amigaos-gcc \
		AR=m68k-amigaos-ar \
		RANLIB=m68k-amigaos-ranlib \
		LD=m68k-amigaos-ld \
		CFLAGS="-noixemul -O2" \
		CPPFLAGS="-noixemul" \
		LDFLAGS="-noixemul" \
		ac_cv_func_fork=no \
		/src/configure --host=m68k-unknown-amigaos --disable-shared --enable-static \
			--disable-dependency-tracking \
			--disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib \
			--disable-lzlib --disable-lrziplib --disable-lz4lib \
			--disable-libseccomp --disable-landlock
	'

echo "==> cross build src/ (magic/ needs a target-runnable file(1), so it is skipped;"
echo "    the native build above provides magic.mgc instead)"
docker run --rm \
	-v "$TOPDIR":/src \
	-v "$BUILD_AMIGA":/build-amiga \
	-w /build-amiga "$IMAGE" bash -lc '
		export PATH=/opt/m68k-amigaos/bin:$PATH
		make -C src -j1 MAGIC="PROGDIR:magic.mgc"
	'

cp "$BUILD_AMIGA/src/file" "$OUT/file"
cp "$BUILD_AMIGA/src/.libs/libmagic.a" "$OUT/libmagic.a"
cp "$BUILD_AMIGA/src/magic.h" "$OUT/magic.h"
cp "$BUILD_NATIVE/magic/magic.mgc" "$OUT/magic.mgc"

echo "==> done: $OUT/file $OUT/libmagic.a $OUT/magic.h $OUT/magic.mgc"
echo "    copy file+magic.mgc to the same directory on the target (PROGDIR:)"
echo "    so the default magic path 'PROGDIR:magic.mgc' resolves. Link"
echo "    libmagic.a into other m68k-amigaos -noixemul -fbaserel programs"
echo "    with '#include \"magic.h\"' and '-lmagic -L<this dir>'."
echo
echo "    NOTE: this target has no working mmap(), so file loads the whole"
echo "    ~11MB magic.mgc into malloc'd RAM on every run. That's fine on an"
echo "    accelerated/expanded Amiga but will fail with an out-of-memory"
echo "    error on a stock, unexpanded machine. If that matters, compile a"
echo "    smaller magic database from a subset of magic/Magdir (file -C -m)"
echo "    and point MAGIC/-m at that instead."
