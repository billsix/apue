#!/bin/sh
# Generate rot13c2.c from rot13c2.c.in by extracting the body of
# translate() from rot13a.c (via fixup.awk) and splicing it in at the
# `/* same as before */` marker.
#
# Usage: gen_rot13c2.sh ROT13C2_C_IN ROT13A_C FIXUP_AWK OUTPUT_C
set -eu

absify() {
  case "$1" in
    /*) printf '%s\n' "$1" ;;
    *)  printf '%s/%s\n' "$PWD" "$1" ;;
  esac
}

C_IN=$(absify "$1")
ROT13A=$(absify "$2")
FIXUP=$(absify "$3")
OUT=$(absify "$4")

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

awk -f "$FIXUP" "$ROT13A" > "$WORK/xlate"
sed '/same/q' "$C_IN"        > "$OUT"
cat "$WORK/xlate"           >> "$OUT"
sed '1,/same/d' "$C_IN"     >> "$OUT"
