#!/usr/bin/env python3
"""Add C23 [[maybe_unused]] to parameters the compiler flags as unused.

Driven by a -Wunused-parameter warning log (the gate's own output), so it only
touches parameters genuinely flagged — these are the required signatures of
signal handlers, thread start routines and callbacks, where the unused
parameter is intentional and the attribute documents that.

Usage:  maybe_unused.py <warnings.log>
Paths in the log are build-relative (../chapter/file.c); resolved against the
apue.3e tree located relative to this script. Idempotent: a parameter that
already carries [[maybe_unused]] is skipped.
"""
import re
import sys
import pathlib

LOG = pathlib.Path(sys.argv[1])
TREE = pathlib.Path(__file__).resolve().parents[3] / "apue.3e"
BUILD = TREE / "build"

PAT = re.compile(
    r"^(\S+?):(\d+):(\d+): warning: .*parameter '([^']+)'.*\[-Wunused-parameter\]",
    re.M,
)

sites: dict[pathlib.Path, list[tuple[int, int, str]]] = {}
for m in PAT.finditer(LOG.read_text()):
    path = (BUILD / m.group(1)).resolve()
    sites.setdefault(path, []).append((int(m.group(2)), int(m.group(3)), m.group(4)))

ATTR = "[[maybe_unused]] "
changed = 0
for path, entries in sites.items():
    if not path.exists():
        print(f"skip (missing): {path}")
        continue
    lines = path.read_text().splitlines(keepends=True)
    # group by line; apply descending column so earlier offsets stay valid
    by_line: dict[int, list[tuple[int, str]]] = {}
    for ln, col, name in entries:
        by_line.setdefault(ln, []).append((col, name))
    for ln, cols in by_line.items():
        text = lines[ln - 1]
        for col, _name in sorted(cols, reverse=True):
            # the parameter declaration starts just after the enclosing '(' or ','
            delim = max(text.rfind("(", 0, col - 1), text.rfind(",", 0, col - 1))
            if delim < 0:
                continue
            ins = delim + 1
            while ins < len(text) and text[ins] == " ":
                ins += 1
            if text[ins:ins + len(ATTR)] == ATTR:
                continue  # already done
            text = text[:ins] + ATTR + text[ins:]
            changed += 1
        lines[ln - 1] = text
    path.write_text("".join(lines))

print(f"[[maybe_unused]] inserted at {changed} sites across {len(sites)} files")
