#!/usr/bin/env bash
# Cross-build libmagic for Amiga systems across every supported CRT/multilib
# slot and install the archives plus magic.h into an SDK tree.
#
#   ./build-amiga-sdk.sh [OS ...]        # default: AmigaOS3 AmigaOS4 MorphOS
#
# SDK location comes from $AMIGA_SDK (default /opt/amiga). Each OS installs
# under its own target subdir (m68k-amigaos, ppc-amigaos, ppc-morphos); an OS
# whose subdir is absent is skipped rather than treated as a failure.
#
# The variant table lives in amiga-variants.sh, shared with package-amiga.sh,
# so both scripts always build the same set. Each slot is built with exactly
# the ABI its directory name means -- libb* dirs get -fbaserel, libm020 gets
# -m68020, and so on. libm060 is deliberately skipped: gcc resolves -m68040
# and -m68060 to the libm020 multilib, so that directory is never searched.
set -uo pipefail

SDK=${AMIGA_SDK:-/opt/amiga}
cd "$(dirname "$0")"
ROOT=$PWD
source ./amiga-variants.sh
OSES=("$@"); [[ ${#OSES[@]} -eq 0 ]] && OSES=(AmigaOS3 AmigaOS4 MorphOS)

[[ -f configure ]] || { echo "error: run autoreconf -i first" >&2; exit 1; }

ok=0; fail=0; failed=()
for os in "${OSES[@]}"; do
  read -r image sdksub host <<<"$(os_image "$os")" || { echo "skip: unknown OS $os"; continue; }
  # /opt/amiga keeps the target tree directly under <target>/, while an
  # AmigaSDK-gcc checkout nests it under <os>/sdk/<target>. Accept either.
  osdir=$(os_sdkdir "$os")
  if   [[ -d $SDK/$sdksub ]];           then SDKROOT=$SDK/$sdksub
  elif [[ -d $SDK/$osdir/$sdksub ]];    then SDKROOT=$SDK/$osdir/$sdksub
  else echo "skip $os: neither $SDK/$sdksub nor $SDK/$osdir/$sdksub"; continue; fi

  echo "=== $os -> $SDKROOT ==="
  eval "vars=(\"\${variants_$os[@]}\")"
  for v in "${vars[@]}"; do
    IFS='|' read -r tree slot cargs <<<"$v"
    bdir="bld-$os-$(echo "$tree$slot" | tr -c 'A-Za-z0-9' '-')"
    printf '  %-7s %-16s ' "$tree" "$slot"

    # Images run as root and ignore USER/GROUP; chown so host `rm -rf` works.
    if ! docker run --rm -v "$ROOT":/work -e USER="$(id -u)" -e GROUP="$(id -g)" "$image" sh -c "
        set -e
        trap 'chown -R $(id -u):$(id -g) /work/$bdir' EXIT
        rm -rf /work/$bdir && mkdir -p /work/$bdir && cd /work/$bdir
        ../configure --host=$host $CONFIGURE_COMMON $cargs CFLAGS='-O2' >configure.log 2>&1
        make -C src magic.h >make-magic-h.log 2>&1
        make -j\$(nproc) -C src libmagic.la >make.log 2>&1
      " 2>/dev/null; then
      echo "BUILD FAILED (logs in $bdir)"; fail=$((fail+1)); failed+=("$os/$tree/$slot"); continue
    fi

    lib=$bdir/src/.libs/libmagic.a
    [[ -f $lib ]] || { echo "no libmagic.a"; fail=$((fail+1)); failed+=("$os/$tree/$slot"); continue; }

    base=$SDKROOT; [[ $tree != "." ]] && base=$SDKROOT/$tree
    dest=$base/lib; [[ $slot != "." ]] && dest=$base/lib/$slot
    mkdir -p "$dest" "$base/include"
    cp "$lib" "$dest/libmagic.a"
    cp "$bdir/src/magic.h" "$base/include/magic.h"
    printf 'ok (%s bytes)\n' "$(wc -c <"$lib" | tr -d ' ')"
    rm -rf "$bdir"; ok=$((ok+1))
  done
done

echo
echo "built+installed: $ok   failed: $fail"
((fail)) && { printf 'failures: %s\n' "${failed[*]}"; exit 1; }
exit 0
