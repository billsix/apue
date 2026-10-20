#!/usr/bin/env bash
# Discover top-of-function declaration blocks in apue.3e.
#
# A "top-of-function declaration block" = one or more consecutive bare
# declaration lines (no initializer, no call) that are the FIRST statements
# inside a function body, i.e. immediately after the line that opens the
# function with `{`. These are the C89-style openers the declare-at-first-use
# rule removes; a mid-block declaration is fine and is NOT reported.
#
# Heuristic, not a parser: it keys off a function-signature line ending in `{`
# at column 0 (the repo is clang-formatted Google style, so definitions open
# `type name(...) {` with the brace on the signature line and the body indented).
# Output is a worklist to eyeball, not an auto-fix input.
#
# Run from anywhere; resolves the repo root itself. Writes the worklist to
# tasks/adhoc/finish-declare-at-use/data/worklist.txt under the repo root.
set -u
ROOT="$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
cd "$ROOT" || exit 1
OUT="tasks/adhoc/finish-declare-at-use/data/worklist.txt"
mkdir -p "$(dirname "$OUT")"

python3 -I - "$@" <<'PY' | tee "$OUT"
import re, subprocess, sys
root = subprocess.check_output(["git", "rev-parse", "--show-toplevel"]).decode().strip()
import pathlib
files = subprocess.check_output(
    ["git", "-C", root, "ls-files", "apue.3e/*.c"]).decode().split()
# A bare declaration line: leading ws, a type, names, optional [..], ';' — and
# crucially NO '=' (uninitialized) and NO '(' (not a call/prototype-as-stmt).
decl = re.compile(
    r'^[ \t]+(?:const\s+|static\s+|register\s+|volatile\s+)*'
    r'(?:unsigned\s+|signed\s+)?'
    r'(?:int|char|long|short|float|double|void|size_t|ssize_t|pid_t|off_t|'
    r'mode_t|time_t|uid_t|gid_t|dev_t|ino_t|nlink_t|socklen_t|sig_atomic_t|'
    r'struct\s+\w+|union\s+\w+|enum\s+\w+|FILE|DIR|[A-Za-z_]\w*_t)\b'
    r'[^=(;]*;[ \t]*$')
# A function-opener line: starts at column 0 (not a label/typedef/struct),
# contains '(' and ')' and ends with '{'. Excludes `} else {` etc.
opener = re.compile(r'^[A-Za-z_].*\).*\{\s*$')
total = 0
for rel in files:
    p = pathlib.Path(root) / rel
    try:
        lines = p.read_text(errors="replace").splitlines()
    except OSError:
        continue
    i = 0
    n = len(lines)
    while i < n:
        if opener.match(lines[i]):
            # collect consecutive bare-decl lines immediately after the opener
            j = i + 1
            block = []
            while j < n and decl.match(lines[j]):
                block.append((j + 1, lines[j].rstrip()))
                j += 1
            if block:
                total += len(block)
                print(f"\n### {rel}:{i+1}  {lines[i].strip()}")
                for ln, text in block:
                    print(f"  {ln}: {text.strip()}")
            i = j
        else:
            i += 1
print(f"\n==== {total} top-of-function declaration lines across the tree ====",
      file=sys.stderr)
PY
