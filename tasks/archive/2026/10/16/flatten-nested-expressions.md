# Flatten nested expressions across the apue example tree (teaching style)

**Status:** **COMPLETE** (2026-10-16). Go-ahead given 2026-10-16 (William Emerison Six
<billsix@gmail.com>); completed the same day.
**Priority:** 7 · **Difficulty:** 6
**Created:** 2026-10-16 (William Emerison Six <billsix@gmail.com>) · **Completed:** 2026-10-16

## BLUF

Applied the teaching flatten-nested-expressions convention
(`tasks/reference/teaching-flatten-expressions.md`) across the `apue.3e/` example tree: every
assignment folded into the condition that tests it was separated into a plain step so the code reads
as plain steps for someone new to C/Unix. Two follow-on readability passes rode along at the
maintainer's request — declarations pulled closer to first use, and a scope-brace survey. **Done** =
every fold removed (except one intentional teaching bug), behavior preserved, the build green under the
warning gate. Oracle: 371 targets compile warning-clean under `-Werror`, the 8 wired `meson test` cases
pass, before and after each pass; `clang-format` run and re-gated green. 68 `.c` files changed.

## Context

The modern-C pass (`tasks/archive/2026/10/16/modernize-to-modern-c.md`) deliberately kept each
program's control-flow structure and changed only types/`const`/idioms; de-nesting changes control-flow
shape, so it was split out as its own reviewed pass with its own go-ahead. The convention it applies
(and the rationale for overriding the shared standard's expression-density idioms *for this teaching
tree*) lives in `tasks/reference/teaching-flatten-expressions.md` — read that first.

## Pass 1 — flatten assignment-in-condition (commit "flatten: …")

Inventory began with `grep -rnE '(while|if|for) *\(\([a-zA-Z_]\w* *= *[^=]' apue.3e --include='*.c'`
(110 sites where the assignment is the first token after `((`), then a hand-scan widened it, because
that leading-token grep misses member/subscript lvalues and non-leading folds. De-nested by shape:

- **Read loops** (`while ((c = getc(stdin)) != EOF)`, `while ((n = read(...)) > 0)`) → loop-and-a-half
  (`for (;;) { T x = f(); if (<stop>) break; … }`), dropping the old function-scope declaration only
  when the counter is local to the loop; kept it where the counter is read after the loop
  (`intro/mycat.c`, `signals/sigtstp.c`, `sockets/ruptime.c`, several `printer/printd.c` loops).
- **Single-shot checks** (`if ((x = f()) < 0)`) → named intermediate; declaration merged when
  single-use, split in place when the variable is reused / loop-carried / in a `setjmp` function /
  address-taken / a `static` / a struct member.
- **Compound tests** split via the negated stop test: `getpass.c` `(c = getc(fp)) != EOF && c != '\n'`
  → `if (c == EOF || c == '\n') break;`; `t_raw.c` `(c &= 255) == 0177` → two statements; `db.c`
  `(c = *ptr++) != 0 && c == SPACE` → a primed `while`.
- **Hand-scan finds beyond the 110** (member/subscript/non-leading folds): `semaph.c`
  `(op_op[0].sem_op = value) == 0`, `bufargs.c` `argv[argc = 0]`, `qid[i] = msgget(...)`
  (`ipc2/pollmsg.c`, `exercises/pollmsg2.c`), nine `db.c` `db->field = …` folds
  (malloc/lseek/atoi/atol), `exercises/fmemopen.c` `ms->flags`/`ms->buf`, `termios/ttyname.c`
  `ddp->d_name = strdup(...)`, and the `cmptr == nullptr && (cmptr = malloc(CONTROLLEN)) == nullptr`
  guard in `lib/sendfd.c` / `lib/recvfd.c` / `lib/Orecvfd.c` / `ipc2/sendfd2.c` / `ipc2/recvfd2.c`
  (→ nested `if`). A final re-grep confirmed zero folds remain except the intentional one.

**Hazards left as-is** (behavior / teaching value):

- `stdio/getcharbug.c` — the folded, `char`-typed `while ((c = getchar()) != EOF)` IS the bug the
  program demonstrates (`char` can't hold `EOF`). Left nested and `char`-typed, with an in-code note so
  a future sweep won't "fix" it.
- `signals/read2.c` — the assignment was split out of the test, but the `int n` declaration was left in
  place because the function uses `setjmp`/`longjmp` (`c-modernization-sweep.md` hazard: don't move a
  local in a `setjmp` function).

**Execution.** `lib/` (33 sites), the generator-source `advio/rot13a.c` (its read loop is in `main()`,
not the `translate()` body that `fixup.awk` splices, so the generator was unaffected), and the two
hazard files were done by hand. The five application-chapter groups (ipc2, printer, proc+intro,
termios/signals/pty/sockets, scattered singletons) were fanned out to five fresh general-purpose
subagents (Edit-only; git/build/format forbidden), with every diff reviewed on disk and gate-verified
afterward — the fan-out pattern recorded in memory from the modernization pass (fresh general-purpose
subagents, not context-inheriting forks).

## Pass 2 — declare-at-use (commit "declare-at-use: …")

The flatten split-in-place sites left a handle variable (`fd`/`fdm`/`fds`/`pid`/`name`/`clifd`/`ptr`/
`len`/`staletime`) declared in a group at the top of a `lib/` socket/pty helper and first-used at the
split assignment. Pulled each down to a `T x = f();` declaration at first use, splitting the grouped
declaration, in `cliconn.c`, `servlisten.c`, `servaccept.c`, `ptyopen.c`, `ptyfork.c`, `daemonize.c`,
`popen.c`. The error-only `err` moved to `errout:  int err = errno;` (a declaration after a label,
legal and clean in C23); the goto-join `rval` (assigned in branches, read at `errout:`) stayed declared
before the branches; address-taken (`stat`, `un`, `statbuf`) and `static` variables stayed in place.

**Left alone deliberately:** the `ipc2/opend/loop.{select,poll}.c` grouped loop-state declarations
(select/poll state reassigned each iteration — fragmenting the cohesive group would hurt readability,
not help), `semaph.c` (`register` + `goto again`), and the application first-assignments the
modernization/flatten already had at use.

## Pass 3 — scope-limiting braces (surveyed, nothing to do)

Surveyed the tree for places where a `{ }` block would usefully limit a variable's scope. The only
real pattern — a declaration inside a `switch` case — already carries its brace where it occurs
(`ipc2/recvfd2.c` `case SCM_CREDTYPE: { … }`); declare-at-use already minimizes scope elsewhere; and
broad scope-brace insertion would add nesting noise that *hurts* the newcomer readability this task
optimizes for. Also checked for deeply nested call-in-condition expressions deserving a named
intermediate — the hits (`write(fd, buf, strlen(buf))`, `fcntl(fd, F_SETOWN, getpid())`, …) are a
single clear outer call with a trivial inner argument, so none warranted change. No edits.

## Verification (oracle, every pass)

- `meson compile` builds all 371 targets clean under `warning_level=2` + the extra flags + `-Werror`.
- The 8 wired `meson test` cases pass (`intro_hello`, `intro_uidgid`, `intro_ls1`, `intro_testerror`,
  `environ_hello1`, `environ_getrlimit`, `threads_exitstatus`, `threads_threadid`). Coverage is partial
  — most programs need a TTY/stdin and aren't auto-tested — so untested programs were kept
  mechanical/behavior-preserving and verified by the green `-Werror` compile; same bar as the
  modernization.
- Gate run against the existing image (no nested `: image` rebuild), then `clang-format` + re-gate.

## Cross-links

- `tasks/reference/teaching-flatten-expressions.md` — the convention + patterns (the durable record).
- `tasks/archive/2026/10/16/modernize-to-modern-c.md` — the prior behavior-preserving pass + oracle.
- `github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md` — the shared standard
  whose expression-density idioms this convention overrides for the example tree.
- `github.com/billsix/runClaudeInContainer:tasks/reference/c-modernization-sweep.md` — the preservation
  hazards (setjmp locals, deliberate teaching bugs) this pass respected.
