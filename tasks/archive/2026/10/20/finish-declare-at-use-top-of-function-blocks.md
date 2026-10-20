# Finish declare-at-use: move the leftover top-of-function declaration blocks

**Status:** done 2026-10-20 (both passes; verified) — staged, awaiting the maintainer's commit, then archive
**Priority:** 5
**Difficulty:** 4 (mechanical per-site, but behavior-preserving; needs a warning-clean rebuild per file)
**Created:** 2026-10-20 (William Emerison Six <billsix@gmail.com>: "in [apue], look at daemonize,
there's a lot of variables declared at the top, and not made local, nor bracketed scope. It could
really use that … add a task to apue to fix that and anything else that looks like it")

## Done (2026-10-20, two passes, fanned out to fresh Edit-only subagents, coordinator-audited)

**Open question resolved** (the scope question below): fixed the clear wins, left variables genuinely
read throughout a whole function, left the preservation hazards — exactly as recommended.

**Pass 1 — declare-at-first-use** (the leftover C89 top-of-function blocks): ~40 files edited across
`lib/`, `ipc1/ipc2/`, `printer/`, `db/`, `threads/threadctl/exercises/`, `proc/pty/termios/sockets/`,
`intro/fileio/stdio/environ/`. Each top-of-function declaration pushed to first use and initialized
(scalars assigned once → `const`; address-taken/output-filled structs moved adjacent to their fill
call with **no** `const`; field-set structs → designated initializers, e.g. `struct sigaction`,
`struct flock`, `struct timeval`). `daemonize.c`/`daemons/init.c` are the exemplar.

**Pass 2 — bare `{ }` phase blocks** (the maintainer's "sweep again … perhaps brackets can be added
to limit scope"): deliberately conservative for a teaching tree. Only **3 files** earned a block —
`daemonize.c` + `daemons/init.c` (the SIGHUP phase's `struct sigaction sa`, which leaked across five
later phases) and `printer/printd.c` (`printer_thread()`'s IPP/HTTP request-construction phase; block
extended through the `writev` that consumes the iovec bases). Everywhere else a block was *not*
warranted — pass 1's declare-at-use had already closed the leak windows, and a block would only
clutter short/single-phase teaching code.

**Preservation hazards left untouched** (with reasons recorded at the time): the setjmp/longjmp
files (`environ/testjmp.c`, `environ/cmd2.c`, `signals/mask.c`, `signals/read2.c`), the deliberate
teaching bug `stdio/getcharbug.c`, `static` function-locals (`lib/strerror.c`, `ipc1/slock.c`,
`lib/semaph.c` `register`), loop-carried accumulators (`loop.select.c` `maxfd`/`maxi`, `printd.c`
relay I/O counters), the thread-held `ipc2/pollmsg.c` `ti[NQ]` (its address outlives any inner
scope), and `ipc2/tshm.c` `shmid` (printed as a stack-address demo before assignment).

**Verification:** `make build` (meson compile + `meson test`) **green under `-Werror`** after pass 1,
and again after pass 2 + `make format` — **8/8 wired tests OK, 0 fail, zero warnings**. Behavior
preservation for the untested majority rests on the mechanical, compiler-checked nature of the moves
(a scope that's too tight fails to compile; the one hazard the compiler can't catch — address-escape —
was opted out of explicitly). One incidental `termios/winch.c` whitespace fix came from `make format`
(a trailing comment left mis-aligned by the earlier `for(;;)`→`while(true)` sweep). Discovery worklog:
`tasks/adhoc/finish-declare-at-use/`.

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

1. ~~Scope of the sweep: fix **every** top-of-function declaration block, or only the clear wins?~~
   **Resolved (maintainer, 2026-10-20): fix the clear wins, leave a declaration read throughout a
   whole function where it is, leave the hazards.** Done exactly that — see the Done record above.

## See also

- `github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md` — "Scope and
  initialization" (the rule + the bare-block phase idiom + why lint doesn't catch it).
- `github.com/billsix/runClaudeInContainer:tasks/reference/c-modernization-sweep.md` — the
  lint-invisible-transforms note (daemonize is the cited example).
- `tasks/archive/2026/10/16/modernize-to-modern-c.md` — the pass that this finishes.
- `tasks/modernization-parity-with-gltron.md` — the sibling remaining-gaps task.
- `tasks/reference/teaching-flatten-expressions.md` — the one place apue overrides the standard
  (expression density), which does NOT affect scope guidance.
