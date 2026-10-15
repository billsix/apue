#!/usr/bin/env bash
#
# Run clang-tidy over the APUE example tree using the meson build's
# compile_commands.json. Portable: runs in-container and on the host from
# anywhere (it locates the apue.3e tree root relative to itself).
#
#   tools/lint.sh                 # lint every .c in the tree (advisory)
#   tools/lint.sh fileio proc     # lint only those chapter dirs
#   tools/lint.sh --fix stdio      # apply clang-tidy fixes to one dir
#   BUILD_DIR=build tools/lint.sh  # override the meson build dir
#
# Exit status is nonzero if clang-tidy reported on any file (so a gate can
# chain it); every file is still processed first (no fail-fast masking).
set -uo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
tree_root="$(dirname "$here")"
cd "$tree_root"

build="${BUILD_DIR:-build}"
if [ ! -f "$build/compile_commands.json" ]; then
  echo "lint: $build/compile_commands.json not found — run 'meson setup $build' first" >&2
  exit 2
fi

fix=()
targets=()
for a in "$@"; do
  case "$a" in
    --fix) fix=(--fix) ;;
    *) targets+=("$a") ;;
  esac
done

files=()
if [ "${#targets[@]}" -eq 0 ]; then
  mapfile -t files < <(find . -name '*.c' -not -path './build/*' | sort)
else
  for t in "${targets[@]}"; do
    if [ -d "$t" ]; then
      mapfile -t -O "${#files[@]}" files < <(find "$t" -name '*.c' | sort)
    else
      files+=("$t")
    fi
  done
fi

status=0
for f in "${files[@]}"; do
  clang-tidy "${fix[@]}" -p "$build" "$f" || status=1
done
exit "$status"
