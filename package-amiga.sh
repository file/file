#!/usr/bin/env bash
# Build and package the Amiga distributions for every supported system.
#
#   ./package-amiga.sh [OS ...]        # default: AmigaOS3 AmigaOS4 MorphOS
#
# Archive layout:
#     magic.readme                         at the archive root
#     magic/C/magic.mgc                    OS-independent, shipped once
#     magic/<OS>/<crt>/{include,lib}/      per-OS libraries and headers
#
#     file.readme                          at the archive root
#     file/<OS>/C/{file,magic.mgc}         a complete drop-in per machine
set -uo pipefail

cd "$(dirname "$0")"
ROOT=$PWD; DIST=$ROOT/dist
source ./amiga-variants.sh
OSES=("$@"); [[ ${#OSES[@]} -eq 0 ]] && OSES=(AmigaOS3 AmigaOS4 MorphOS)

command -v lha >/dev/null || { echo "error: lha not found on PATH" >&2; exit 1; }
[[ -f configure ]] || { echo "error: run autoreconf -i first" >&2; exit 1; }
MGC=$ROOT/build-native/magic/magic.mgc
[[ -f $MGC ]] || { echo "error: $MGC missing -- build build-native first" >&2; exit 1; }

# The compiler images run as root. Their entrypoint is `exec "$@"` and
# ignores USER/GROUP, so give the tree back to the host user on the way
# out -- otherwise the later `rm -rf` prints a page of Permission denied.
uid=$(id -u); gid=$(id -g)
build() {  # image host extra-args bdir target
  local image=$1 host=$2 extra=$3 bdir=$4 target=$5
  docker run --rm -v "$ROOT":/work -e USER="$uid" -e GROUP="$gid" "$image" sh -c "
      set -e
      trap 'chown -R $uid:$gid /work/$bdir' EXIT
      rm -rf /work/$bdir && mkdir -p /work/$bdir && cd /work/$bdir
      ../configure --host=$host $CONFIGURE_COMMON $extra CFLAGS='-O2' >configure.log 2>&1
      make -C src magic.h >make-magic-h.log 2>&1
      make -j\$(nproc) -C src $target >make.log 2>&1
    " 2>/dev/null
}

rm -rf "$DIST"; mkdir -p "$DIST"
SL=$DIST/stage-magic; SF=$DIST/stage-file
mkdir -p "$SL/magic/C"
cp "$MGC" "$SL/magic/C/magic.mgc"
cp "$ROOT/magic.readme" "$SL/magic.readme"
cp "$ROOT/file.readme"     "$SF/file.readme" 2>/dev/null || { mkdir -p "$SF"; cp "$ROOT/file.readme" "$SF/file.readme"; }

libs=0; apps=0; failed=()
for os in "${OSES[@]}"; do
  read -r image sdksub host <<<"$(os_image "$os")" || { echo "skip: unknown OS $os"; continue; }
  echo "=== $os ==="
  eval "vars=(\"\${variants_$os[@]}\")"
  for v in "${vars[@]}"; do
    IFS='|' read -r tree slot cargs <<<"$v"
    bdir="pkg-$os-$(echo "$tree$slot" | tr -c 'A-Za-z0-9' '-')"
    printf '  magic    %-7s %-16s ' "$tree" "$slot"
    if ! build "$image" "$host" "$cargs" "$bdir" libmagic.la; then
      echo "FAILED (logs in $bdir)"; failed+=("$os/$tree/$slot"); continue
    fi
    base=$SL/magic/$os; [[ $tree != "." ]] && base=$SL/magic/$os/$tree
    dest=$base/lib; [[ $slot != "." ]] && dest=$base/lib/$slot
    mkdir -p "$dest" "$base/include"
    cp "$bdir/src/.libs/libmagic.a" "$dest/libmagic.a"
    cp "$bdir/src/magic.h" "$base/include/magic.h"
    echo "ok"; libs=$((libs+1)); rm -rf "$bdir"
  done

  eval "appargs=\${app_args_$os}"
  printf '  file     (most compatible)         '
  if build "$image" "$host" "$appargs" "pkg-app-$os" file; then
    mkdir -p "$SF/file/$os/C"
    cp "pkg-app-$os/src/file" "$SF/file/$os/C/file"
    cp "$MGC" "$SF/file/$os/C/magic.mgc"
    echo "ok"; apps=$((apps+1)); rm -rf "pkg-app-$os"
  else
    echo "FAILED (logs in pkg-app-$os)"; failed+=("$os/file")
  fi
done

# NOTE: build the archive from a clean staging tree with `lha -aq2`. Do NOT try
# to refresh a single entry (e.g. the readme) in an existing archive with
# `lha u` -- it APPENDS a second entry with the same name rather than replacing
# it, and extraction then yields the stale copy.
( cd "$SL" && lha -aq2 "$DIST/magic.lha" * >/dev/null ) || failed+=("magic.lha")
( cd "$SF" && lha -aq2 "$DIST/file.lha" * >/dev/null )  || failed+=("file.lha")
rm -rf "$SL" "$SF"
echo
echo "magic variants: $libs    file binaries: $apps"
ls -la "$DIST"
((${#failed[@]})) && { printf 'failed: %s\n' "${failed[*]}"; exit 1; }
exit 0
