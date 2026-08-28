dnl Process this file with autoconf to produce a configure script.
AC_INIT([file],[5.48],[christos@astron.com])
AM_INIT_AUTOMAKE([subdir-objects foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_MSG_CHECKING(for builtin ELF support)
AC_ARG_ENABLE(elf,
[  --disable-elf            disable builtin ELF support],
[if test "${enableval}" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE([BUILTIN_ELF], 1, [Define if built-in ELF support is used])
else
  AC_MSG_RESULT(no)
fi], [
  # enable by default
  AC_MSG_RESULT(yes)
  AC_DEFINE([BUILTIN_ELF], 1, [Define in built-in ELF support is used])
])

AC_MSG_CHECKING(for ELF core file support)
AC_ARG_ENABLE(elf-core,
[  --disable-elf-core       disable ELF core file support],
[if test "${enableval}" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE([ELFCORE], 1, [Define for ELF core file support])
else
  AC_MSG_RESULT(no)
fi], [
  # enable by default
  AC_MSG_RESULT(yes)
  AC_DEFINE([ELFCORE], 1, [Define for ELF core file support])
])

AC_MSG_CHECKING(for zlib support)
AC_ARG_ENABLE([zlib],
[AS_HELP_STRING([--disable-zlib], [disable zlib compression support @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_zlib)

AC_MSG_CHECKING(for bzlib support)
AC_ARG_ENABLE([bzlib],
[AS_HELP_STRING([--disable-bzlib], [disable bz2lib compression support @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_bzlib)

AC_MSG_CHECKING(for xzlib support)
AC_ARG_ENABLE([xzlib],
[AS_HELP_STRING([--disable-xzlib], [disable liblzma/xz compression support @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_xzlib)

AC_MSG_CHECKING(for zstdlib support)
AC_ARG_ENABLE([zstdlib],
[AS_HELP_STRING([--disable-zstdlib], [disable zstdlib compression support @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_zstdlib)

AC_MSG_CHECKING(for lzlib support)
AC_ARG_ENABLE([lzlib],
[AS_HELP_STRING([--disable-lzlib], [disable liblz (lzip) compression support @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_lzlib)

AC_MSG_CHECKING(for lrziplib support)
AC_ARG_ENABLE([lrziplib],
[AS_HELP_STRING([--disable-lrziplib], [disable liblrzip (lrzip) compression support @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_lrziplib)

AC_MSG_CHECKING(for lz4 support)
AC_ARG_ENABLE([lz4lib],
[AS_HELP_STRING([--disable-lz4lib], [disable liblz4 (lz4) compression support @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_lz4kklib)


AC_MSG_CHECKING(for libseccomp support)
AC_ARG_ENABLE([libseccomp],
[AS_HELP_STRING([--disable-libseccomp], [disable libseccomp sandboxing @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_libseccomp)

AC_MSG_CHECKING(for Landlock LSM support)
AC_ARG_ENABLE([landlock],
[AS_HELP_STRING([--disable-landlock], [disable Landlock filesystem sandboxing @<:@default=auto@:>@])])
AC_MSG_RESULT($enable_landlock)

AC_MSG_CHECKING(for file formats in man section 5)
AC_ARG_ENABLE(fsect-man5,
[  --enable-fsect-man5      enable file formats in man section 5],
[if test "${enableval}" = yes; then
  AC_MSG_RESULT(yes)
  fsect=5
else
  AC_MSG_RESULT(no)
  fsect=4
fi], [
  # disable by default
  AC_MSG_RESULT(no)
  fsect=4
])

AC_CANONICAL_HOST
case "$host_os" in
   mingw32*)
      MINGW=1
      ;;
   *)
      MINGW=0
      ;;
esac
AC_SUBST(MINGW)
AM_CONDITIONAL(MINGW, test "$MINGW" = 1)

dnl ---------------------------------------------------------------------------
dnl AmigaOS m68k ABI selection
dnl
dnl Mirrors the M68K_CRT / M68K_CPU / M68K_BASEREL options of the json-c Amiga
dnl port. These three flags change the ABI, so they are appended to $CC right
dnl after AC_PROG_CC below -- that way every feature test in this file runs with
dnl the same flags the library is finally built with, which matters because the
dnl CRT choice selects a different set of target headers.
dnl ---------------------------------------------------------------------------
AC_ARG_WITH([m68k-crt],
  [AS_HELP_STRING([--with-m68k-crt=CRT],
    [AmigaOS m68k C runtime: newlib, nix20, nix13, ixemul or clib2 @<:@default=newlib@:>@])],
  [m68k_crt=$withval], [m68k_crt=newlib])
AC_ARG_WITH([m68k-cpu],
  [AS_HELP_STRING([--with-m68k-cpu=CPU],
    [AmigaOS m68k CPU target, e.g. 68000, 68020, 68040, 68060 @<:@default=compiler default@:>@])],
  [m68k_cpu=$withval], [m68k_cpu=])
AC_ARG_WITH([m68k-baserel],
  [AS_HELP_STRING([--with-m68k-baserel=MODE],
    [AmigaOS m68k base-relative addressing: baserel, baserel32 or off @<:@default=off@:>@])],
  [m68k_baserel=$withval], [m68k_baserel=off])

dnl Which Amiga flavour are we targeting? --host=m68k-*-amigaos and
dnl --host=ppc-amigaos both normalise to host_os=amigaos, so split on the CPU.
AMIGA_TARGET=
case "$host_os:$host_cpu" in
  amigaos*:m68k*)                 AMIGA_TARGET=m68k ;;
  amigaos*:powerpc*|amigaos*:ppc*) AMIGA_TARGET=os4 ;;
  morphos*:*)                     AMIGA_TARGET=morphos ;;
esac

AC_ARG_WITH([ppc-crt],
  [AS_HELP_STRING([--with-ppc-crt=CRT],
    [AmigaOS 4 C runtime: newlib, clib2 or clib4 @<:@default=newlib@:>@])],
  [ppc_crt=$withval], [ppc_crt=newlib])
AC_ARG_WITH([morphos-clib],
  [AS_HELP_STRING([--with-morphos-clib=LIB],
    [MorphOS C library: default or libnix @<:@default=default@:>@])],
  [morphos_clib=$withval], [morphos_clib=default])
AC_ARG_WITH([morphos-baserel32],
  [AS_HELP_STRING([--with-morphos-baserel32],
    [MorphOS: build with -mbaserel32 @<:@default=no@:>@])],
  [morphos_baserel32=$withval], [morphos_baserel32=no])

AMIGA_M68K_ABI_FLAGS=
AMIGA_M68K_EXTRA_LIBS=

case "$AMIGA_TARGET" in
  os4)
    case "$ppc_crt" in
      newlib) ;;
      clib2|clib4) AMIGA_M68K_ABI_FLAGS="-mcrt=$ppc_crt" ;;
      *) AC_MSG_ERROR([--with-ppc-crt must be newlib, clib2 or clib4 (got: $ppc_crt)]) ;;
    esac
    ;;
  morphos)
    case "$morphos_clib" in
      default) ;;
      libnix)  AMIGA_M68K_ABI_FLAGS="-mclib=libnix" ;;
      *) AC_MSG_ERROR([--with-morphos-clib must be default or libnix (got: $morphos_clib)]) ;;
    esac
    if test "x$morphos_baserel32" = xyes; then
      AMIGA_M68K_ABI_FLAGS="$AMIGA_M68K_ABI_FLAGS -mbaserel32"
    fi
    ;;
esac

case "$host_os" in
  amigaos*)
    if test "x$AMIGA_TARGET" != xm68k; then :; else
    case "$m68k_crt" in
      newlib|nix20|nix13|ixemul)
        AMIGA_M68K_ABI_FLAGS="-mcrt=$m68k_crt" ;;
      clib2)
        dnl clib2 keeps its POSIX layer (getopt_long, utime, ...) in libunix,
        dnl which is not linked by default. Without it configure mis-detects
        dnl those functions as missing and builds replacements that then clash
        dnl with clib2's own prototypes. Applied together with the ABI flags
        dnl after AC_PROG_CC -- adding it any earlier makes AC_PROG_CC's own
        dnl link test fail, since -mcrt=clib2 is not in $CC yet.
        AMIGA_M68K_ABI_FLAGS="-mcrt=$m68k_crt"
        AMIGA_M68K_EXTRA_LIBS="-lunix" ;;
      *)
        AC_MSG_ERROR([--with-m68k-crt must be newlib, nix20, nix13, ixemul or clib2 (got: $m68k_crt)]) ;;
    esac

    case "$m68k_cpu" in
      "")   ;;
      -m*)  AMIGA_M68K_ABI_FLAGS="$AMIGA_M68K_ABI_FLAGS $m68k_cpu" ;;
      m*)   AMIGA_M68K_ABI_FLAGS="$AMIGA_M68K_ABI_FLAGS -$m68k_cpu" ;;
      *)    AMIGA_M68K_ABI_FLAGS="$AMIGA_M68K_ABI_FLAGS -m$m68k_cpu" ;;
    esac

    dnl -fbaserel32 is only available on 68020 and later.
    case "$m68k_baserel" in
      off) ;;
      baserel)
        AMIGA_M68K_ABI_FLAGS="$AMIGA_M68K_ABI_FLAGS -fbaserel" ;;
      baserel32)
        case "$m68k_cpu" in
          ""|*68000*|*68010*)
            AC_MSG_ERROR([--with-m68k-baserel=baserel32 requires --with-m68k-cpu=68020 or later]) ;;
        esac
        AMIGA_M68K_ABI_FLAGS="$AMIGA_M68K_ABI_FLAGS -fbaserel32" ;;
      *)
        AC_MSG_ERROR([--with-m68k-baserel must be baserel, baserel32 or off (got: $m68k_baserel)]) ;;
    esac
    fi
    ;;
esac

AC_SUBST([pkgdatadir], ['$(datadir)/misc'])
AC_SUBST(fsect)
AM_CONDITIONAL(FSECT5, test x$fsect = x5)

AC_SUBST(WARNINGS)

dnl Checks for programs.
AC_PROG_CC
if test -n "$AMIGA_M68K_ABI_FLAGS"; then
  CC="$CC $AMIGA_M68K_ABI_FLAGS"
  test -n "$AMIGA_M68K_EXTRA_LIBS" && LIBS="$LIBS $AMIGA_M68K_EXTRA_LIBS"
  AC_MSG_NOTICE([Amiga target=$AMIGA_TARGET flags:$AMIGA_M68K_ABI_FLAGS $AMIGA_M68K_EXTRA_LIBS])
fi
AC_USE_SYSTEM_EXTENSIONS
AM_PROG_CC_C_O
AC_C_BIGENDIAN
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT([disable-static pic-only])
gl_VISIBILITY
dnl Checks for headers
AC_HEADER_MAJOR
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(stdint.h fcntl.h inttypes.h unistd.h byteswap.h)
AC_CHECK_HEADERS(spawn.h utime.h wchar.h wctype.h limits.h)
AC_CHECK_HEADERS(getopt.h err.h xlocale.h)
AC_CHECK_HEADERS(sys/bswap.h sys/mman.h sys/stat.h sys/types.h sys/utime.h sys/time.h sys/sysmacros.h sys/ioctl.h)
if test "$enable_zlib" != "no"; then
  AC_CHECK_HEADERS(zlib.h)
fi
if test "$enable_bzlib" != "no"; then
  AC_CHECK_HEADERS(bzlib.h)
fi
if test "$enable_xzlib" != "no"; then
  AC_CHECK_HEADERS(lzma.h)
fi
if test "$enable_zstdlib" != "no"; then
  AC_CHECK_HEADERS(zstd.h zstd_errors.h)
fi
if test "$enable_lzlib" != "no"; then
  AC_CHECK_HEADERS(lzlib.h)
fi
if test "$enable_lrziplib" != "no"; then
  AC_CHECK_HEADERS(Lrzip.h)
fi
if test "$enable_lz4lib" != "no"; then
  AC_CHECK_HEADERS(lz4frame.h)
fi
AC_CHECK_TYPE([sig_t],[AC_DEFINE([HAVE_SIG_T],1,[Have sig_t type])],,[#include <signal.h>])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_CHECK_MEMBERS([struct stat.st_flags])

AC_CHECK_MEMBERS([struct tm.tm_gmtoff],,,[#include <time.h>])
AC_STRUCT_TIMEZONE
AC_STRUCT_TIMEZONE_DAYLIGHT
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_TYPE_MBSTATE_T

AC_STRUCT_OPTION_GETOPT_H
AC_TYPE_PID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_INT32_T
AC_TYPE_UINT64_T
AC_TYPE_INT64_T
AC_TYPE_INTPTR_T
AC_TYPE_UINTPTR_T
m4_ifdef([AC_SYS_YEAR2038], [AC_SYS_YEAR2038], [
# GNU libc only allows setting _TIME_BITS when FILE_OFFSET_BITS is also set.
# GNU libc defines __TIMESIZE on systems where _TIME_BITS can be set.
AS_IF([test X"$ac_cv_sys_file_offset_bits" = X"64"], [
    AC_CHECK_DECL(__TIMESIZE, [
	AC_DEFINE([_TIME_BITS], [64], [Number of bits in a timestamp, on hosts where this is settable.])
    ], [], [
AC_INCLUDES_DEFAULT
#include <time.h>
	])
    ])
])
AC_FUNC_MMAP
AC_FUNC_FORK
AC_FUNC_MBRTOWC

AC_MSG_CHECKING(for gcc compiler warnings)
AC_ARG_ENABLE(warnings,
[  --disable-warnings	disable compiler warnings],
[if test "${enableval}" = no -o "$GCC" = no; then
   AC_MSG_RESULT(no)
   WARNINGS=
else
   AC_MSG_RESULT(yes)
   WARNINGS="-Wall -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith \
       -Wmissing-declarations -Wredundant-decls -Wnested-externs \
       -Wsign-compare -Wreturn-type -Wswitch -Wshadow \
       -Wcast-qual -Wwrite-strings -Wextra -Wunused-parameter -Wformat=2"
fi], [
if test "$GCC" = yes; then
   AC_MSG_RESULT(yes)
   WARNINGS="-Wall -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith \
       -Wmissing-declarations -Wredundant-decls -Wnested-externs \
       -Wsign-compare -Wreturn-type -Wswitch -Wshadow \
       -Wcast-qual -Wwrite-strings -Wextra -Wunused-parameter -Wformat=2"
else
   WARNINGS=
   AC_MSG_RESULT(no)
fi])

dnl newlib for m68k-amigaos ships a <dirent.h> that is a hard #error, so this
dnl must be a compile test rather than a presence test.
AC_CHECK_HEADERS([dirent.h])
AC_CHECK_HEADERS([regex.h])

dnl Checks for functions
AC_CHECK_FUNCS(strndup mkstemp mkostemp utimes utime wcwidth strtof newlocale uselocale freelocale memmem pipe pipe2 posix_spawnp qsort_r select sigaction pathconf fork ioctl readlink tzset)

dnl MorphOS links fork() but does not prototype it, so the presence of the
dnl function and the presence of its declaration have to be probed separately.
AC_CHECK_DECLS([fork])

dnl Provide implementation of some required functions if necessary
AC_CONFIG_LIBOBJ_DIR([src/missing])
AC_REPLACE_FUNCS(getopt_long asprintf vasprintf strlcpy strlcat getline ctime_r asctime_r localtime_r gmtime_r pread strcasestr fmtcheck dprintf reallocarray)

dnl Some targets (e.g. AmigaOS libnix) have no POSIX regex implementation.
AC_REPLACE_FUNCS(regcomp regexec regerror regfree)

dnl Checks for libraries
if test "$enable_zlib" != "no"; then
  AC_CHECK_LIB(z, gzopen)
fi
if test "$enable_bzlib" != "no"; then
  AC_CHECK_LIB(bz2, BZ2_bzCompressInit)
fi
if test "$enable_xzlib" != "no"; then
  AC_CHECK_LIB(lzma, lzma_stream_decoder)
fi
if test "$enable_zstdlib" != "no"; then
  AC_CHECK_LIB(zstd, ZSTD_createDStream)
fi
if test "$enable_lzlib" != "no"; then
  AC_CHECK_LIB(lz, LZ_decompress_open)
fi
if test "$enable_lrziplib" != "no"; then
  AC_CHECK_LIB(lrzip, lrzip_new)
fi
if test "$enable_lz4lib" != "no"; then
  AC_CHECK_LIB(lz4, LZ4F_createDecompressionContext)
fi

if test "$enable_libseccomp" != "no"; then
    AC_CHECK_LIB(seccomp, seccomp_init)
fi
if test "$enable_landlock" != "no"; then
    AC_CHECK_HEADERS(linux/landlock.h)
fi
if test "$MINGW" = 1; then
  AC_CHECK_LIB(gnurx,regexec,,AC_MSG_ERROR([libgnurx is required to build file(1) with MinGW]))
fi

dnl See if we are cross-compiling
AM_CONDITIONAL(IS_CROSS_COMPILE, test "$cross_compiling" = yes)

dnl Final sanity checks
if test "$enable_zlib" = "yes"; then
  if test "$ac_cv_header_zlib_h$ac_cv_lib_z_gzopen" != "yesyes"; then
    AC_MSG_ERROR([zlib support requested but not found])
  fi
fi
if  test "$ac_cv_header_zlib_h$ac_cv_lib_z_gzopen" = "yesyes"; then
  AC_DEFINE([ZLIBSUPPORT], 1, [Enable zlib compression support])
fi
if test "$enable_bzlib" = "yes"; then
  if test "$ac_cv_header_bzlib_h$ac_cv_lib_bz2_BZ2_bzCompressInit" != "yesyes"; then
    AC_MSG_ERROR([bzlib support requested but not found])
  fi
fi
if  test "$ac_cv_header_bzlib_h$ac_cv_lib_bz2_BZ2_bzCompressInit" = "yesyes"; then
  AC_DEFINE([BZLIBSUPPORT], 1, [Enable bzlib compression support])
fi
if test "$enable_xzlib" = "yes"; then
  if test "$ac_cv_header_lzma_h$ac_cv_lib_lzma_lzma_stream_decoder" != "yesyes"; then
    AC_MSG_ERROR([xzlib support requested but not found])
  fi
fi
if  test "$ac_cv_header_lzma_h$ac_cv_lib_lzma_lzma_stream_decoder" = "yesyes"; then
  AC_DEFINE([XZLIBSUPPORT], 1, [Enable xzlib compression support])
fi
if test "$enable_zstdlib" = "yes"; then
  if test "$ac_cv_header_zstd_h$ac_cv_lib_zstd_ZSTD_createDStream" != "yesyes"; then
    AC_MSG_ERROR([zstdlib support requested but not found])
  fi
fi
if  test "$ac_cv_header_zstd_h$ac_cv_lib_zstd_ZSTD_createDStream" = "yesyes"; then
  AC_DEFINE([ZSTDLIBSUPPORT], 1, [Enable zstdlib compression support])
fi
if test "$enable_lzlib" = "yes"; then
  if test "$ac_cv_header_lzlib_h$ac_cv_lib_lz_LZ_decompress_open" != "yesyes"; then
    AC_MSG_ERROR([lzlib support requested but not found])
  fi
fi
if  test "$ac_cv_header_lzlib_h$ac_cv_lib_lz_LZ_decompress_open" = "yesyes"; then
  AC_DEFINE([LZLIBSUPPORT], 1, [Enable lzlib compression support])
fi

if test "$enable_lrziplib" = "yes"; then
  if test "$ac_cv_header_Lrzip_h$ac_cv_lib_lrzip_lrzip_new" != "yesyes"; then
    AC_MSG_ERROR([lrzip support requested but not found])
  fi
fi
if  test "$ac_cv_header_Lrzip_h$ac_cv_lib_lrzip_lrzip_new" = "yesyes"; then
  AC_DEFINE([LRZIPLIBSUPPORT], 1, [Enable lrziplib compression support])
fi

if test "$enable_lz4lib" = "yes"; then
  if test "$ac_cv_header_lz4frame_h$ac_cv_lib_lz4_LZ4F_createDecompressionContext" != "yesyes"; then
    AC_MSG_ERROR([lz4 support requested but not found])
  fi
fi
if test "$ac_cv_header_lz4frame_h$ac_cv_lib_lz4_LZ4F_createDecompressionContext" = "yesyes"; then
  AC_DEFINE([LZ4LIBSUPPORT], 1, [Enable lz4 compression support])
fi

AC_CONFIG_FILES([Makefile src/Makefile magic/Makefile tests/Makefile doc/Makefile python/Makefile libmagic.pc])
AC_OUTPUT
