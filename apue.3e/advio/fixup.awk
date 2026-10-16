#!/usr/bin/awk -f
# Extract the statement body (no braces) of translate() from rot13a.c so
# gen_rot13c2.sh can splice it into rot13c2.c.in. Robust to the brace-on-the-
# same-line layout clang-format produces (the old /^translate/ + /^{/ matcher
# assumed K&R layout and silently extracted nothing once rot13a.c was
# reformatted, yielding an empty translate() and a -Wreturn-type bug).
BEGIN { ready = 0 }
# Opening line of translate(): names the function and ends with '{'. The guard
# on a trailing '{' skips the later call site "buf[i] = translate(buf[i]);".
/translate[ \t]*\(/ && /\{[ \t]*$/ { ready = 1; next }
# A '}' at column 0 closes the function.
ready && /^\}/ { ready = 0; next }
ready { print }
