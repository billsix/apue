#!/bin/sh
# Generate standards/conf.c (or options.c) by running an awk script
# that reads two .sym files from the *current directory*. We stage the
# .sym sources into a temp dir under the names the awk script expects
# (sysconf.sym + pathconf.sym, or sysopt.sym + pathopt.sym), filter out
# comment lines (`grep -v '^#'`), then run awk there and capture stdout.
#
# Usage: gen_standards_c.sh OUT_C AWK_SCRIPT \
#                           SYM_LIM_SOURCE SYM_LIM_NAME \
#                           PATH_LIM_SOURCE PATH_LIM_NAME
set -eu

absify() {
  case "$1" in
    /*) printf '%s\n' "$1" ;;
    *)  printf '%s/%s\n' "$PWD" "$1" ;;
  esac
}

OUT=$(absify "$1")
AWK_SCRIPT=$(absify "$2")
SYM_SRC=$(absify "$3")
SYM_NAME=$4
PATH_SRC=$(absify "$5")
PATH_NAME=$6

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

grep -v '^#' "$SYM_SRC"  > "$WORK/$SYM_NAME"
grep -v '^#' "$PATH_SRC" > "$WORK/$PATH_NAME"

(cd "$WORK" && awk -f "$AWK_SCRIPT") > "$OUT"
