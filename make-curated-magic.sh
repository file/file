#!/usr/bin/env bash
# Compile a reduced magic database for embedding in an application.
#
#   ./make-curated-magic.sh [outfile]        # default: dist/magic-curated.mgc
#
# The full magic.mgc is 10.8 MB, and libmagic reads all of it into memory at
# magic_load() time -- too much to ask of a 68k Amiga. This builds a database
# from just the Magdir sources needed to recognise the file types an LLM API
# will accept, plus the native Amiga formats, which comes to about 2.4 MB (~100
# KB once lha-compressed).
#
# Needs the native build (build-native/) for the file(1) that compiles it.
set -euo pipefail

cd "$(dirname "$0")"
OUT=${1:-dist/magic-curated.mgc}
IMAGE=sacredbanana/amiga-compiler:m68k-amigaos

# images: png/gif/webp/bmp/tiff...   jpeg: jpeg    pdf: pdf
# zip: zip/jar   msooxml: docx/xlsx/pptx   msdos: OLE doc/xls/ppt
# rtf: rtf   riff: wav/avi   audio: mp3 and friends   sgml: html/xml
# iff+amigaos: ILBM, 8SVX, hunk executables, and other native formats
# elf: AmigaOS 4 and MorphOS executables
SOURCES=(images jpeg pdf zip msooxml msdos rtf riff audio sgml iff amigaos elf)

[[ -x build-native/src/file ]] || { echo "error: build build-native first" >&2; exit 1; }
mkdir -p "$(dirname "$OUT")"

docker run --rm -v "$PWD":/work "$IMAGE" sh -c "
    set -e
    rm -rf /tmp/curated && mkdir -p /tmp/curated
    for f in ${SOURCES[*]}; do cp /work/magic/Magdir/\$f /tmp/curated/; done
    cd /tmp && /work/build-native/src/file -C -m curated
    cp /tmp/curated.mgc /work/$OUT
  "

printf 'wrote %s (%s bytes) from %d magic sources\n' \
    "$OUT" "$(wc -c <"$OUT" | tr -d ' ')" "${#SOURCES[@]}"
