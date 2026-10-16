# Teaching style — flatten nested expressions for C/Unix newcomers

**Reference document.** Read before writing, modernizing, or reviewing the example programs under
`apue.3e/`. States a deliberate pedagogical convention for this codebase. Durable; update in place.
(Written 2026-10-16 — William Emerison Six <billsix@gmail.com>.)

## The convention

apue is a **teaching codebase** for people new to C and Unix. Favor **flat statements over nested
expressions** — in particular, do **not** fold an assignment into the condition that tests it. A
student reads two plain steps ("get a byte, then test it") more easily than one combined condition.

The canonical case is the read loop. The textbook/idiomatic form nests the assignment inside the test:

```c
int c;
while ((c = getc(stdin)) != EOF) {
  if (putc(c, stdout) == EOF)
    err_sys("output error");
}
```

`(c = getc(stdin)) != EOF` means **two** things: `c = getc(stdin)`, then test `c != EOF`. Split them.
The clearest de-nested form is a loop-and-a-half — one read per iteration, an explicit EOF test, no
duplicated read:

```c
for (;;) {
  int c = getc(stdin);
  if (c == EOF)
    break;
  if (putc(c, stdout) == EOF)
    err_sys("output error");
}
```

Acceptable alternatives (same idea — assignment is its own step, the condition is a plain compare):
a `for (int c = getc(stdin); c != EOF; c = getc(stdin))` loop, or a primed `c = getc(stdin);` before a
`while (c != EOF) { … c = getc(stdin); }`. Pick whichever reads most plainly for the program at hand.

More generally: give a deeply nested sub-expression a **named intermediate** (`ssize_t n = read(…);`
then test `n`), rather than burying the call inside the condition or another call.

## Why this is a deliberate choice (and what it overrides)

The maintainer (a Lisp programmer) finds nested expressions natural and personally prefers the
textbook form — this convention is **not** a claim that the nested form is wrong. It is a choice to
optimize *this* codebase for a reader learning C, the same way `example-c-teaching-style` reasoning is
applied elsewhere in the fleet. It therefore **overrides, for the `apue.3e/` example tree, the shared
standard's expression-density idioms** (github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md
— "prefer expressions", EAFP, comprehension-style density). Everything else in the shared standard
still applies.

## Scope and correctness

- Applies to the **`apue.3e/` example programs** (and `lib/` where it aids clarity). `musl/` is out of
  scope (vendored).
- **Behavior must be preserved** — this is a source-shape change only; the wired `meson test` cases and
  a green compile are the oracle (same bar as the modernization,
  `tasks/archive/2026/10/16/modernize-to-modern-c.md`). Watch EOF/`getchar()` edge cases when
  de-nesting a read loop (a `char` vs `int` for `c` is a real bug — keep `int`).

## Status

The tree currently still uses the nested idiom (it was modernized for types/const/etc., not de-nested).
Applying this convention across the examples is tracked in `tasks/flatten-nested-expressions.md`
(proposed) — until then, follow it in any **new** or **touched** example code.

## Cross-links

- `tasks/flatten-nested-expressions.md` — the tree-wide application task (proposed).
- `tasks/archive/2026/10/16/modernize-to-modern-c.md` — the modern-C pass (behavior-preservation oracle).
- github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md — the shared standard this
  overrides for the example tree.
