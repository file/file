## README for file(1) Command and the libmagic(3) library ##

    @(#) $File: README.md,v 1.8 2026/08/28 15:26:23 christos Exp $

- Bug Tracker: <https://bugs.astron.com/>
- Build Status: <https://travis-ci.org/file/file>
- Download link: <ftp://ftp.astron.com/pub/file/>
- E-mail: <christos@astron.com>
- Fuzzing link: <https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=1&q=proj:file>
- Home page: https://www.darwinsys.com/file/
- Mailing List archives: <https://mailman.astron.com/pipermail/file/>
- Mailing List: <file@astron.com>
- Public repo: <https://github.com/file/file>
- Test framework: <https://github.com/file/file-tests>

Phone: Do not even think of telephoning me about this program. Send
cash first!

This is Release 5.x of Ian Darwin's (copyright but distributable)
file(1) command, an implementation of the Unix File(1) command.
It knows the 'magic number' of several thousands of file types.
This version is the standard "file" command for Linux, *BSD, and
other systems. (See "patchlevel.h" for the exact release number).

The major changes for 5.x are CDF file parsing, indirect magic,
name/use (recursion) and overhaul in mime and ascii encoding
handling.

The major feature of 4.x is the refactoring of the code into a
library, and the re-write of the file command in terms of that
library. The library itself, libmagic can be used by 3rd party
programs that wish to identify file types without having to fork()
and exec() file. The prime contributor for 4.0 was Mans Rullgard.

UNIX is a trademark of UNIX System Laboratories.

The prime contributor to Release 3.8 was Guy Harris, who put in
megachanges including byte-order independence.

The prime contributor to Release 3.0 was Christos Zoulas, who put
in hundreds of lines of source code changes, including his own
ANSIfication of the code (I liked my own ANSIfication better, but
his (__P()) is the "Berkeley standard" way of doing it, and I wanted
UCB to include the code...), his HP-like "indirection" (a feature
of the HP file command, I think), and his mods that finally got
the uncompress (-z) mode finished and working.

This release has compiled in numerous environments; see PORTING
for a list and problems.

This fine freeware file(1) follows the USG (System V) model of the
file command, rather than the Research (V7) version or the V7-derived
4.[23] Berkeley one. That is, the file /etc/magic contains much of
the ritual information that is the source of this program's power.
My version knows a little more magic (including tar archives) than
System V; the /etc/magic parsing seems to be compatible with the
(poorly documented) System V /etc/magic format (with one exception;
see the man page).

In addition, the /etc/magic file is built from a subdirectory
for easier(?) maintenance.  I will act as a clearinghouse for
magic numbers assigned to all sorts of data files that
are in reasonable circulation. Send your magic numbers,
in magic(5) format please, to the maintainer, Christos Zoulas.

COPYING - read this first.
* `ChangeLog` - log of important changes
* `README.md` - read this second (you are currently reading this file).
* `INSTALL` - read on how to install
* `src/apprentice.c` - parses /etc/magic to learn magic
* `src/apptype.c` - used for OS/2 specific application type magic
* `src/ascmagic.c` - third & last set of tests, based on hardwired assumptions.
* `src/asctime_r.c` - replacement for OS's that don't have it.
* `src/asprintf.c` - replacement for OS's that don't have it.
* `src/buffer.c` - buffer handling functions.
* `src/cdf.[ch]` - parser for Microsoft Compound Document Files
* `src/cdf_time.c` - time converter for CDF.
* `src/compress.c` - handles decompressing files to look inside.
* `src/ctime_r.c` - replacement for OS's that don't have it.
* `src/der.[ch]` - parser for Distinguished Encoding Rules
* `src/dprintf.c` - replacement for OS's that don't have it.
* `src/elfclass.h` - common code for elf 32/64.
* `src/encoding.c` - handles unicode encodings
* `src/file.c` - the main program
* `src/file.h` - header file
* `src/file_opts.h` - list of options
* `src/fmtcheck.c` - replacement for OS's that don't have it.
* `src/fsmagic.c` - first set of tests the program runs, based on filesystem info
* `src/funcs.c` - utilility functions
* `src/getline.c` - replacement for OS's that don't have it.
* `src/getopt_long.c` - replacement for OS's that don't have it.
* `src/gmtime_r.c` - replacement for OS's that don't have it.
* `src/is_csv.c` - knows about Comma Separated Value file format (RFC 4180).
* `src/is_json.c` - knows about JavaScript Object Notation format (RFC 8259).
* `src/is_simh.c` - knows about SIMH tape file format.
* `src/is_tar.c, tar.h` - knows about Tape ARchive format (courtesy John Gilmore).
* `src/landlock.c` - linux landlock protection
* `src/localtime_r.c` - replacement for OS's that don't have it.
* `src/magic.h.in` - source file for magic.h
* `src/mygetopt.h` - replacement for OS's that don't have it.
* `src/magic.c` - the libmagic api
* `src/names.h` - header file for ascmagic.c
* `src/pread.c` - replacement for OS's that don't have it.
* `src/print.c` - print results, errors, warnings.
* `src/readcdf.c` - CDF wrapper.
* `src/readelf.[ch]` - Stand-alone elf parsing code.
* `src/softmagic.c` - 2nd set of tests, based on /etc/magic
* `src/swap.h` - byte swapping
* `src/swap.c` - byte swapping
* `src/mygetopt.h` - replacement for OS's that don't have it.
* `src/seccomp.c` - linux seccomp protection
* `src/strcasestr.c` - replacement for OS's that don't have it.
* `src/strlcat.c` - replacement for OS's that don't have it.
* `src/strlcpy.c` - replacement for OS's that don't have it.
* `src/strndup.c` - replacement for OS's that don't have it.
* `src/tar.h` - tar file definitions
* `src/vasprintf.c` - for systems that don't have it.
* `doc/file.man` - man page for the command
* `doc/magic.man` - man page for the magic file, courtesy Guy Harris.
	Install as magic.4 on USG and magic.5 on V7 or Berkeley; cf Makefile.

Magdir - directory of /etc/magic pieces
------------------------------------------------------------------------------

If you submit a new magic entry please make sure you read the following
guidelines:

- Initial match is preferably at least 32 bits long, and is a _unique_ match
- If this is not feasible, use additional check
- Match of <= 16 bits are not accepted
- Delay printing string as much as possible, don't print output too early
- Avoid printf arbitrary byte as string, which can be a source of
  crash and buffer overflow

- Provide complete information with entry:
  * One line short summary
  * Optional long description
  * File extension, if applicable
  * Full name and contact method (for discussion when entry has problem)
  * Further reference, such as documentation of format

gpg for dummies:
------------------------------------------------------------------------------

```
$ gpg --verify file-X.YY.tar.gz.asc file-X.YY.tar.gz
gpg: assuming signed data in `file-X.YY.tar.gz'
gpg: Signature made WWW MMM DD HH:MM:SS YYYY ZZZ using DSA key ID KKKKKKKK
```

To download the key:

```
$ gpg --keyserver hkp://keys.gnupg.net --recv-keys KKKKKKKK
```
------------------------------------------------------------------------------


Parts of this software were developed at SoftQuad Inc., developers
of SGML/HTML/XML publishing software, in Toronto, Canada.
SoftQuad was swallowed up by Corel in 2002 and does not exist any longer.

Building for Amiga systems
--------------------------

`libmagic` and `file` cross-compile for AmigaOS 3 (m68k), AmigaOS 4 (PPC) and
MorphOS (PPC) using the `sacredbanana/amiga-compiler` toolchain images:

| system | image tag | host triplet |
|--------|-----------|--------------|
| AmigaOS 3 | `sacredbanana/amiga-compiler:m68k-amigaos` | `m68k-unknown-amigaos` |
| AmigaOS 4 | `sacredbanana/amiga-compiler:ppc-amigaos` | `ppc-amigaos` |
| MorphOS | `sacredbanana/amiga-compiler:ppc-morphos` | `ppc-morphos` |

### Scripts

Three scripts at the repository root drive this; run `autoreconf -i` once first.

* `amiga-variants.sh` -- the shared table of every variant that is built, one
  entry per `tree|slot|configure args`, plus the `configure` flags common to
  all of them. Both scripts below source it, so they can never disagree.
* `build-amiga-sdk.sh [OS ...]` -- builds every variant and installs
  `libmagic.a` plus `magic.h` into an SDK tree. The location comes from
  `$AMIGA_SDK` (default `/opt/amiga`); either a `<target>/` or an
  AmigaSDK-gcc-style `amigaos3/sdk/<target>/` layout is accepted, and an OS
  whose subdirectory is absent is skipped rather than failing.
* `package-amiga.sh [OS ...]` -- builds every variant plus one `file` binary
  per OS and produces the two distribution archives in `dist/`.

Both default to all three systems. The archives are laid out as:

```
magic.readme
magic/C/magic.mgc                       shared -- it is not machine code
magic/AmigaOS3/{libnix,clib2}/{include,lib/...}
magic/AmigaOS4/{include,lib/clib4}
magic/MorphOS/{include,lib,lib/libb32,lib/libnix,lib/libb32/libnix}

file.readme
file/{AmigaOS3,AmigaOS4,MorphOS}/C/{file,magic.mgc}
```

`package-amiga.sh` needs a native `magic.mgc` to ship, so build the host copy
into `build-native/` first; it is refused otherwise.

Note that m68k and PPC lay their C runtimes out differently, and the variant
table follows each. On m68k a runtime is a separate tree (`libnix/`, `clib2/`),
each with its own `include/` and `lib/`. On PPC a runtime is a multilib *slot*
beneath a single shared `lib/` -- `-mcrt=clib4` searches `<target>/lib/clib4/`
and takes headers from `<target>/include/`. This was read off
`ppc-amigaos-gcc -mcrt=clib4 -print-search-dirs` rather than assumed, because
installing to the wrong one produces a library the compiler silently never
finds.

### Continuous integration

`.github/workflows/amiga-packages.yml` runs the whole packaging path on every
push to `master` (and on demand via *Run workflow*), so the commit gets a tick
when both archives build and verify and a cross when anything fails. It
uploads `magic.lha` and `file.lha` as run artifacts, named `magic-lha` and
`file-lha`; GitHub always wraps an artifact in a zip, so the download is a zip
containing the untouched `.lha`.

The runner builds *LHa for UNIX* (`jca02266/lha`, pinned to a release tag) from
source, because Debian and Ubuntu only ship `lhasa`, which extracts `.lha` but
cannot create one. After packaging, the workflow lists both archives and checks
the counts inside them -- one `libmagic.a` per variant, one `magic.h` per
per-OS tree, one `file` and one `magic.mgc` per OS, one copy of each readme --
with the expected numbers derived from `amiga-variants.sh` so the check cannot
go stale when a slot is added. That matters because a build failure now makes
`package-amiga.sh` exit non-zero, but a missing copy would otherwise leave a
perfectly valid, quietly incomplete archive.

### Building by hand

```
docker run --rm -v ${PWD}:/work -e USER=$( id -u ) -e GROUP=$( id -g ) \
    -it sacredbanana/amiga-compiler:m68k-amigaos bash
mkdir build-amiga && cd build-amiga
../configure --host=m68k-unknown-amigaos --disable-shared --enable-static \
    --disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib \
    --disable-lzlib --disable-lrziplib --disable-lz4lib \
    --disable-libseccomp --disable-landlock CFLAGS="-O2"
make -C src magic.h && make -C src libmagic.la
```

`libmagic.a` is left in `build-amiga/src/.libs/`, and the public header
`magic.h` is generated into `build-amiga/src/`.

### ABI options

These change the ABI, so they are applied to the compiler driver *before* any
of `configure`'s feature tests run -- which matters because the C runtime
choice selects a different set of target headers, and a probe run with the
wrong ones produces a wrong `config.h`. `configure` classifies the target from
`--host` and only honours the options for that system.

**AmigaOS 3 (m68k)**

* `--with-m68k-crt=` picks the C runtime, and therefore which multilib the
  library belongs to:

  | value | runtime |
  |-------|---------|
  | `newlib` (default) | newlib |
  | `nix20` | libnix, AmigaOS 2.0+ (this is what `-noixemul` selects) |
  | `nix13` | libnix, AmigaOS 1.3 |
  | `ixemul` | ixemul |
  | `clib2` | clib2 |

* `--with-m68k-cpu=` sets the CPU target, passed through as `-m<cpu>`, e.g.
  `68000`, `68020`, `68040`, `68060`. Omitted means the compiler default.

* `--with-m68k-baserel=` selects base-relative addressing: `baserel`
  (`-fbaserel`), `baserel32` (`-fbaserel32`) or `off`. **The default is `off`.**
  `baserel32` requires `--with-m68k-cpu=68020` or later and is rejected
  otherwise.

**AmigaOS 4 (PPC)**

* `--with-ppc-crt=` selects `newlib` (default), `clib2` or `clib4`. Anything
  other than `newlib` is passed as `-mcrt=<value>`.

**MorphOS (PPC)**

* `--with-morphos-clib=` selects `default` (ixemul) or `libnix`
  (`-mclib=libnix`). The latter is the same multilib `-noixemul` selects, and
  is the self-contained ABI -- an ixemul build needs `ixemul.library` on the
  target machine.
* `--with-morphos-baserel32=yes` adds `-mbaserel32`.

```
../configure --host=m68k-unknown-amigaos --with-m68k-crt=nix20 --with-m68k-cpu=68020
../configure --host=m68k-unknown-amigaos --with-m68k-crt=clib2 --with-m68k-cpu=68020 \
    --with-m68k-baserel=baserel32
../configure --host=ppc-amigaos --with-ppc-crt=clib4
../configure --host=ppc-morphos --with-morphos-baserel32=yes
```

### A warning about base-relative addressing

A base-relative library keeps its globals at an offset from the `a4` register,
and `a4` is established once by the program's startup code. Such a library is
only safe inside a program that is **itself** base-relative *and* that declares
every function the OS can call back into -- BOOPSI/MUI dispatchers, `struct
Hook` entries, interrupt servers -- with `__saveds`, so that `a4` is
re-established on entry. Note that the m68k `SDI_hook.h` macros (`DISPATCHER`,
`HOOKPROTO*`) set up register arguments but do **not** add `__saveds`.

Without that, a hook entered from the OS runs with a foreign `a4`, and every
`a4`-relative access made from it reads from a wrong address -- including
libnix's `malloc`, whose heap state is `a4`-relative. The usual symptom is
intermittent memory corruption and crashes during teardown.

Conversely, a library built *without* `-fbaserel` and compiled at `-O1` or above
may allocate `a4` as a scratch register. That is harmless in a program that is
also non-base-relative, but it corrupts the base pointer of one that is.

In short: match `--with-m68k-baserel` to the consuming program, and prefer `off`
unless you know the program is base-relative and `__saveds`-clean.

### C runtime support status

Verified by building each variant and then linking a program that calls
`magic_open`/`magic_load`/`magic_file` against the resulting `libmagic.a`.

**AmigaOS 3** (gcc 6.5.0b) -- 10 variants shipped:

| runtime | builds | links | notes |
|---------|--------|-------|-------|
| `nix20` (libnix) | all 5 slots | all 5 slots | fully working |
| `clib2` | all 5 slots | 2 of 5 | see below |
| `newlib` | yes | **no** | not shipped, see below |

`clib2` needs `-lunix` **after** `-lmagic`, because its POSIX layer lives
there. The three slots that do not link are gaps in clib2 itself -- a plain
clib2 program links fine in every case:

* `-m68000` (`lib/`, `lib/libb/`): clib2's 68000 multilib does not provide
  `strtoull`, which libmagic references from `apprentice.c`, `print.c` and
  `softmagic.c`.
* `-fbaserel` (`lib/libb/`, `lib/libb/libm020/`): the magic tables push the
  data segment past the 64 KB that 16-bit base-relative addressing allows, and
  clib2's own `libc.a(stdlib_constructor_begin.o)` overflows with *relocation
  truncated to fit: DREL16 against `__DTOR_LIST__`*. Use `-fbaserel32`
  (`lib/libb32/libm020/`) instead, which links cleanly -- `-fbaserel32` has no
  64 KB limit.

So the usable clib2 slots are `lib/libm020/` and `lib/libb32/libm020/`.

`newlib` compiles but cannot be linked: the m68k-amigaos newlib tree defines
none of `access`, `dup2`, `fcntl`, `fstat`, `lstat`, `pipe`, `readlink`, `stat`
or `unlink`. Since stat(2) is central to what libmagic does, newlib support
needs those implemented first and is not shipped.

**AmigaOS 4** (gcc 11.5.0) -- 1 variant shipped:

| runtime | status |
|---------|--------|
| `clib4` | shipped, and it links |
| `newlib`, `clib2` | not shipped -- no `<regex.h>` |

`src/file.h` includes `<regex.h>` unconditionally, and the bundled BSD regex
supplies the four functions but no header of its own. A C runtime shipping no
`<regex.h>` therefore cannot build libmagic at all. The m68k and MorphOS trees
each have one -- with the implementations absent, which `AC_REPLACE_FUNCS` then
fills in from the bundled sources -- but on AmigaOS 4 only clib4 ships the
header, so `newlib` and `clib2` fail at `file.h:82` before anything else is
reached. Re-checked after the `reallocarray` fix below, in case that had been
the real blocker; it was not. Unblocking them means vendoring a `<regex.h>` to
accompany the bundled implementation.

**MorphOS** (gcc 15.2.0) -- 4 variants shipped, all of which link:

| slot | ABI |
|------|-----|
| `lib/` | ixemul |
| `lib/libb32/` | ixemul, `-mbaserel32` |
| `lib/libnix/` | native C library (`-noixemul`) |
| `lib/libb32/libnix/` | native C library, `-mbaserel32` |

The native ABI needs no `ixemul.library` on the target, so it is what the
shipped `file` binary uses and what new code should prefer.

Getting the native ABI to work turned out to be the same bug twice. Because
the C runtime flags are applied to `CC` before the feature tests, an ixemul
configure run correctly reports `pread`, `regcomp` and `reallocarray` as
present -- and a `-noixemul` run correctly reports them absent, at which point
`AC_REPLACE_FUNCS` supplies all three. What it could not fix by itself was
that the bundled regex sources are standalone: they do not include `file.h`,
so they never saw the `reallocarray()` prototype and `regcomp.c` failed with
an implicit declaration. Declaring it in `regex2.h`, which both `regcomp.c`
and `regexec.c` include, fixed the native build and the `libnix` variant that
had been failing the same way.

### AmigaOS notes

libnix on m68k has no POSIX regex implementation, so `configure` falls
back to the bundled BSD `regcomp`/`regexec`/`regerror`/`regfree` (see
`src/REGEX-COPYRIGHT`). They are compiled with `-DLIBHACK=1`, which disables the
GNU regex extensions those sources would otherwise expect, and needed
`__RCSID` stubbed out since AmigaOS `<sys/cdefs.h>` does not define it.

Several things upstream assumes are universal are absent on one runtime or
another, and are now probed by `configure` and guarded in the sources:

* `sigaction` -- provided by **no** AmigaOS runtime. The SIGPIPE suppression in
  `compress.c` is guarded on `HAVE_SIGACTION`; without it a decompressor child
  dying unexpectedly is not shielded against, which only matters when external
  decompressors are enabled.
* `fork` -- declared but not linkable on MorphOS, so `compress.c` now probes
  both the function and its declaration and falls back to `fork() == -1`.
* `ioctl`, `readlink`, `tzset` -- absent from the MorphOS native C library,
  and previously called unconditionally. `ioctl` only ever guarded the
  `FIONREAD` pipe-size probe in `compress.c`, so that block now requires
  `HAVE_IOCTL` too; without `readlink` the `S_IFLNK` arm of `fsmagic.c` cannot
  resolve a link and is compiled out, letting symlinks fall through to the
  generic handling; `tzset` in `print.c` is simply skipped.
* `pathconf` -- absent from newlib. The `PIPE_BUF` fallback in `magic.c` is
  guarded on `HAVE_PATHCONF` and otherwise uses 512.
* `pipe` -- absent from clib2. `file_pipe_closexec()` reports failure rather
  than failing to link, disabling only the external-decompressor path.
* `dirent.h` -- newlib ships one that is a hard `#error`, so it is now a
  compile test and the magic-*directory* loading path in `apprentice.c` is
  guarded on `HAVE_DIRENT_H`. Loading a compiled `.mgc` file is unaffected.
* `utimbuf` -- the use site in `magic.c` was guarded on the *header* macros
  while the include was guarded on `HAVE_UTIME`, so a runtime with the header
  but no `utime()` (newlib) got an incomplete type. Both now agree.
* `regex2.h` needed an explicit `<stdint.h>`; only libnix pulled it in
  transitively.

Because none of these runtimes can spawn an external decompressor, inspecting
a compressed file by shelling out is compiled out everywhere. Reading
compressed content directly is unaffected.
