#!/usr/bin/env bash
#
# Finish the NULL -> nullptr (C23) conversion across apue.3e for the files the
# AST codemod (clang-tidy modernize-use-nullptr) skipped — it is unreliable on
# sources compiled into multiple meson targets.
#
# Only operator-adjacent patterns are rewritten; by construction none of them
# occur in the "/* for ... NULL */" include annotations or the prose comments
# that legitimately mention NULL (verified 2026-10-15). All variadic exec
# sentinels are already (char *)0, so a bare nullptr can never land in varargs.
# Idempotent: a second run finds no operator-adjacent NULL and changes nothing.
#
# Run from anywhere; operates on <repo-root>/apue.3e, skipping build/.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
tree="$root/apue.3e"

mapfile -t files < <(find "$tree" \( -name '*.c' -o -name '*.h' \) \
                       -not -path "$tree/build/*" | sort)

for f in "${files[@]}"; do
  sed -i -E \
    -e 's/== NULL/== nullptr/g' \
    -e 's/!= NULL/!= nullptr/g' \
    -e 's/= NULL;/= nullptr;/g' \
    -e 's/= NULL\)/= nullptr)/g' \
    -e 's/\(NULL\)/(nullptr)/g' \
    -e 's/\(NULL,/(nullptr,/g' \
    -e 's/, NULL\)/, nullptr)/g' \
    -e 's/, NULL,/, nullptr,/g' \
    -e 's/return NULL;/return nullptr;/g' \
    -e 's/\? NULL :/? nullptr :/g' \
    -e 's/: NULL;/: nullptr;/g' \
    -e 's/: NULL\)/: nullptr)/g' \
    "$f"
  # The nullptr keyword needs no header, so the NULL macro is gone from code.
  # Fix the now-stale include annotations: trim NULL from multi-item lists, and
  # drop <stddef.h> lines that existed only for NULL (none of these files use
  # size_t/offsetof — the build gate confirms).
  sed -i -E \
    -e 's|, NULL \*/| */|' \
    -e 's|/\* for NULL, |/* for |' \
    -e '/^#include <stddef\.h>[[:space:]]*\/\* for NULL \*\/$/d' \
    "$f"
done

echo "nullptr codemod applied to ${#files[@]} files"
