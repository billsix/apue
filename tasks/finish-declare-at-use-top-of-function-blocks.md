# Finish declare-at-use: move the leftover top-of-function declaration blocks

**Status:** proposed — needs go-ahead
**Priority:** 5
**Difficulty:** 4 (mechanical per-site, but behavior-preserving; needs a warning-clean rebuild per file)
**Created:** 2026-10-20 (William Emerison Six <billsix@gmail.com>: "in [apue], look at daemonize,
there's a lot of variables declared at the top, and not made local, nor bracketed scope. It could
really use that … add a task to apue to fix that and anything else that looks like it")

## BLUF

The 2026-10-16 modernize-to-modern-C pass (`tasks/archive/2026/10/16/modernize-to-modern-c.md`)
applied declare-at-first-use to the easy cases — loop counters, obvious new locals — but **left
C89-style declaration blocks at the top of some functions**, especially struct-typed locals used
across several phases. `lib/daemonize.c` is the exemplar: its body already uses `nullptr` and
`for (rlim_t i = …)`, yet the function still opens with
`int fd0, fd1, fd2; struct rlimit rl; struct sigaction sa;`. **"Done" = every such top-of-function
declaration block in `apue.3e/` is either pushed to the variable's first use (initialized there) or
scoped into a bare `{ }` phase block, with each program still compiling warning-clean under
`-Werror` and its wired `meson test` (where any) still passing.** Behavior-preserving only.

Why the modernization pass missed it: **no clang-tidy/Cppcheck check moves a declaration to its
first use** (`readability-isolate-declaration` only splits a comma declaration;
`cppcoreguidelines-init-variables` only forces initialization), so these are invisible to the
"is it lint-clean?" gate. The shared standard now says this explicitly — see
`github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md` ("Scope and
initialization": the top-of-function block is a finding even after a modernization pass; grep for it
and judge by eye) and `c-modernization-sweep.md` ("Two high-value transforms are lint-invisible").

## Context (read first)

- **The governing rule is the shared standard's "Scope and initialization — declare late, initialize
  always"**: declare each variable at its first use; the narrowest scope that works is the right one;
  when a variable is read across phases, introduce a bare `{ }` phase block (with a one-line comment)
  rather than leave it at the top. apue's own `CLAUDE.md` adopts the standard (declare-at-first-use is
  listed) and pins C23.
- **apue overrides the standard on expression density only** (the teaching-flatten-expressions rule,
  `tasks/reference/teaching-flatten-expressions.md`); scope/declaration guidance is NOT overridden, so
  it applies in full here.
- **The exemplar, `lib/daemonize.c`** (as of 2026-10-20):
  - `int fd0, fd1, fd2;` (top) → used only at the "attach fd 0/1/2 to /dev/null" phase; declare-and-
    initialize there: `int fd0 = open("/dev/null", O_RDWR); int fd1 = dup(0); int fd2 = dup(0);` (also
    splits the one-per-line comma declaration the standard flags).
  - `struct rlimit rl;` (top) → first used by `getrlimit(RLIMIT_NOFILE, &rl)` and again in the
    fd-closing loop; declare it at the `getrlimit` call (it legitimately spans two phases, so either
    declare at first use or wrap the "get limit … close all fds" span in a bare block).
  - `struct sigaction sa;` (top) → used only at the "ignore SIGHUP" phase; declare it there, and this
    is a good spot for a **designated initializer**: `struct sigaction sa = { .sa_handler = SIG_IGN,
    .sa_flags = 0 };` then `sigemptyset(&sa.sa_mask);` (sa_mask still needs `sigemptyset`).
  - Behavior is identical; the wired tests + a `-Werror` compile are the oracle (per the modernize
    task's contract).
- **Deciding scope is by reading, not grep alone** — a variable that looks single-phase may be read
  later; confirm each move compiles.

## Plan

1. **Discover (read-only).** Grep `apue.3e/` (NOT `apue.3e/build/`, NOT the vendored `musl/`) for
   top-of-function declaration blocks — the lines between a function's opening `{` and its first
   statement that are bare declarations. A useful first cut:
   `grep -rnE '^\s+(const\s+)?(unsigned\s+|signed\s+)?(int|char|long|short|size_t|ssize_t|pid_t|struct\s+\w+|FILE|[A-Za-z_]+_t)\b[^=;(]*;\s*$' apue.3e --include=*.c | grep -v '/build/'`
   then open each hit and judge whether it opens a function (vs. a legitimate mid-block declaration).
   Log the worklist to `tasks/adhoc/finish-declare-at-use/data/`.
2. **Fix each, behavior-preserving.** Push each declaration to its first use (initialized); where a
   variable spans phases, add a bare `{ }` phase block with a one-line comment (the standard's idiom).
   Apply a designated initializer where the struct is set field-by-field right after declaration
   (e.g. `struct sigaction`). One file per staged unit.
3. **Verify per file:** `meson compile -C build` warning-clean under `-Werror`, and `meson test -C
   build` still green for any wired program. State per file what went unverified (most programs are
   untested — behavior preservation rests on the mechanical nature of the move + the clean compile).
4. **Record** the sweep as a work log and cross-link the parity task.

## Not in this task

- **`for (;;)` → `while (true)`** across apue is being handled separately (the same 2026-10-20
  request), as a direct sweep, not here.
- Broader modernization gaps (double-`const`, `-Wpedantic`/`-Wsign-conversion`, gcc lane,
  `[[nodiscard]]`/`[[fallthrough]]`, remaining `sprintf`) live in
  `tasks/modernization-parity-with-gltron.md` — this task is only the declaration-scope finish.

## Open questions

1. Scope of the sweep: fix **every** top-of-function declaration block found, or only the clear wins
   (daemonize and its obvious siblings) and list borderline ones (a variable genuinely used in most
   of a function, where moving it buys little) for a per-site call? Recommend: fix the clear ones,
   leave a declaration that is read throughout the whole function where it is, and list the borderline
   set.

## See also

- `github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md` — "Scope and
  initialization" (the rule + the bare-block phase idiom + why lint doesn't catch it).
- `github.com/billsix/runClaudeInContainer:tasks/reference/c-modernization-sweep.md` — the
  lint-invisible-transforms note (daemonize is the cited example).
- `tasks/archive/2026/10/16/modernize-to-modern-c.md` — the pass that this finishes.
- `tasks/modernization-parity-with-gltron.md` — the sibling remaining-gaps task.
- `tasks/reference/teaching-flatten-expressions.md` — the one place apue overrides the standard
  (expression density), which does NOT affect scope guidance.
