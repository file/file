# Shared variant table for the Amiga build/package scripts.
# Each entry: "tree|slot|configure args"
#   tree/slot are relative to the per-OS SDK root; "." means the top level.

os_image() { case "$1" in
  AmigaOS3) echo "sacredbanana/amiga-compiler:m68k-amigaos m68k-amigaos m68k-unknown-amigaos" ;;
  AmigaOS4) echo "sacredbanana/amiga-compiler:ppc-amigaos ppc-amigaos ppc-amigaos" ;;
  MorphOS)  echo "sacredbanana/amiga-compiler:ppc-morphos ppc-morphos ppc-morphos" ;;
  *) return 1 ;;
esac; }

# Where an AmigaSDK-gcc-style checkout keeps each system's target tree.
os_sdkdir() { case "$1" in
  AmigaOS3) echo "amigaos3/sdk" ;;
  AmigaOS4) echo "amigaos4/sdk" ;;
  MorphOS)  echo "morphos/sdk" ;;
  *) return 1 ;;
esac; }

variants_AmigaOS3=(
  "libnix|.|--with-m68k-crt=nix20 --with-m68k-cpu=68000 --with-m68k-baserel=off"
  "libnix|libm020|--with-m68k-crt=nix20 --with-m68k-cpu=68020 --with-m68k-baserel=off"
  "libnix|libb|--with-m68k-crt=nix20 --with-m68k-cpu=68000 --with-m68k-baserel=baserel"
  "libnix|libb/libm020|--with-m68k-crt=nix20 --with-m68k-cpu=68020 --with-m68k-baserel=baserel"
  "libnix|libb32/libm020|--with-m68k-crt=nix20 --with-m68k-cpu=68020 --with-m68k-baserel=baserel32"
  "clib2|.|--with-m68k-crt=clib2 --with-m68k-cpu=68000 --with-m68k-baserel=off"
  "clib2|libm020|--with-m68k-crt=clib2 --with-m68k-cpu=68020 --with-m68k-baserel=off"
  "clib2|libb|--with-m68k-crt=clib2 --with-m68k-cpu=68000 --with-m68k-baserel=baserel"
  "clib2|libb/libm020|--with-m68k-crt=clib2 --with-m68k-cpu=68020 --with-m68k-baserel=baserel"
  "clib2|libb32/libm020|--with-m68k-crt=clib2 --with-m68k-cpu=68020 --with-m68k-baserel=baserel32"
)
# AmigaOS 4. Unlike m68k, where each C runtime is its own tree (libnix/,
# clib2/), the PPC toolchains make the runtime a multilib SLOT under lib/ --
# clib4 is found in <target>/lib/clib4/, and headers come from the shared
# <target>/include/. Verified with `ppc-amigaos-gcc -mcrt=clib4 -print-search-dirs`.
#
# newlib ships no <regex.h>, which file.h needs; src/regex-bsd.h supplies the
# public header for the bundled BSD implementation so newlib builds anyway.
# clib2 is still omitted: its <getopt.h> declares getopt_long with a different
# prototype than the AC_REPLACE_FUNCS replacement, so getopt_long.c will not
# compile against it.
variants_AmigaOS4=(
  ".|.|"
  ".|clib4|--with-ppc-crt=clib4"
)

# MorphOS ships both ABIs. -mclib=libnix and -noixemul select the SAME multilib
# (verified with -print-search-dirs), and it is the modern, self-contained one --
# an ixemul build needs ixemul.library on the target. Slot names come straight
# from the search dirs:
#   <default>                  -> lib/
#   -mbaserel32                -> lib/libb32/
#   -mclib=libnix              -> lib/libnix/
#   -mclib=libnix -mbaserel32  -> lib/libb32/libnix/
variants_MorphOS=(
  ".|.|"
  ".|libb32|--with-morphos-baserel32=yes"
  ".|libnix|--with-morphos-clib=libnix"
  ".|libb32/libnix|--with-morphos-clib=libnix --with-morphos-baserel32=yes"
)

# The most compatible file(1) per OS.
app_args_AmigaOS3="--with-m68k-crt=nix20 --with-m68k-cpu=68000 --with-m68k-baserel=off"
app_args_AmigaOS4="--with-ppc-crt=clib4"
app_args_MorphOS="--with-morphos-clib=libnix"   # self-contained; no ixemul.library needed

# Must stay on ONE line: it is expanded inside a `sh -c` string, where embedded
# newlines would split it into separate commands.
CONFIGURE_COMMON="--disable-shared --enable-static --disable-dependency-tracking --disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib --disable-lzlib --disable-lrziplib --disable-lz4lib --disable-libseccomp --disable-landlock"
