# Flatten nested expressions across the apue example tree (teaching style)

**Status:** proposed — needs go-ahead
**Priority:** 7 · **Difficulty:** 6
**Created:** 2026-10-16 (William Emerison Six <billsix@gmail.com>)

## BLUF

Apply the de-nesting teaching convention (`tasks/reference/teaching-flatten-expressions.md`) across the
`apue.3e/` example programs: separate assignments from the tests they are folded into (the
`while ((c = getc(stdin)) != EOF)` family) and give deeply nested sub-expressions named intermediates,
so the code reads as plain steps for someone new to C. **Behavior-preserving** — the wired `meson test`
cases + a green compile are the oracle, exactly as in the modern-C pass. A source-shape change only.

## Why not done with the modernization

The modern-C pass (`tasks/archive/2026/10/16/modernize-to-modern-c.md`) deliberately kept each
program's structure and only changed types/`const`/idioms; de-nesting changes control-flow shape, so
it is its own reviewed pass with its own go-ahead.

## Plan (when approved)

1. Inventory the assignment-in-condition sites: `grep -rnE 'while *\(\(|if *\(\(' apue.3e --include='*.c'`
   (the `(c = getc(...)) != EOF` / `(n = read(...)) > 0` family), plus hand-scan for deeply nested
   call-in-condition expressions.
2. De-nest each per the reference doc's patterns (loop-and-a-half preferred for read loops); keep `c`
   an `int` (the `char`/EOF bug), preserve EOF/error handling exactly.
3. Verify per chapter: green compile under the warning gate + the 8 wired `meson test` cases; untested
   programs stay mechanical/behavior-preserving (state what went unverified, per the modernization).
4. Do it in reviewable chunks (per chapter), not one blast.

## Constraints

- `apue.3e/` examples (+ `lib/` where it helps); **`musl/` out of scope**.
- In-container verification (`make build` / the gate), same as the modernization task.

## Cross-links

- `tasks/reference/teaching-flatten-expressions.md` — the convention + patterns (read first).
- `tasks/archive/2026/10/16/modernize-to-modern-c.md` — the prior behavior-preserving pass + oracle.
