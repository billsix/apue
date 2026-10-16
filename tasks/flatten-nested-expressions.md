# Flatten nested expressions across the apue example tree (teaching style)

**Status:** in progress — go-ahead given 2026-10-16 (William Emerison Six <billsix@gmail.com>)
**Priority:** 7 · **Difficulty:** 6
**Created:** 2026-10-16 (William Emerison Six <billsix@gmail.com>)

## Inventory (2026-10-16)

110 assignment-in-condition sites across 21 chapters (`grep -rnE '(while|if|for) *\(\([a-zA-Z_]\w* *= *[^=]'
apue.3e --include='*.c'`). Three shapes:

- **Read loop** `while ((c = getc(stdin)) != EOF)` / `while ((n = read(...)) > 0)` → loop-and-a-half
  (`for (;;) { T x = f(); if (<stop>) break; … }`). Compound loop test (`… && c != '\n'`) folds into the
  `if (<stop>)` via De Morgan.
- **Single-shot error check** `if ((x = f()) < 0)` → named intermediate on its own line, then plain compare.
  Merge the declaration (`T x = f();`) when the variable is single-use; keep a separate `x = f();` when the
  variable is reused/loop-carried or declared in a `setjmp` function.
- **`realloc`/`getXent`/`readdir`/`strtok` `== nullptr`** → same named-intermediate split.

### Hazard sites (handled specially, not mechanically)

- `stdio/getcharbug.c` — **deliberate teaching bug** (`char c` can't hold `getchar()`/`EOF`). The folded
  buggy idiom IS the lesson → **opt out, leave the nested form**, add an in-code note so a future sweep
  won't "fix" it.
- `signals/read2.c` — `int n` is a local in a **`setjmp`/`longjmp`** function → split the assignment but
  **do not move/merge the declaration** (`c-modernization-sweep.md` hazard).

## Progress

**Flatten pass — DONE (2026-10-16), gate green.** All assignment-in-condition sites de-nested across
the `apue.3e/` tree (68 `.c` files). Oracle: 371 targets compile warning-clean under `-Werror`, 8/8
`meson test` cases pass, before and after; then `clang-format` run and re-gated green. Behavior
preserved everywhere.

- **Mechanism per shape:** read loops → loop-and-a-half (`for (;;) { T x = f(); if (<stop>) break; }`),
  removing the old function-scope declaration only when the counter is local to the loop (kept it where
  the counter is read after the loop, e.g. `intro/mycat.c`, `signals/sigtstp.c`, `sockets/ruptime.c`);
  single-shot checks → named intermediate, merging the declaration when single-use and splitting in
  place when the variable is reused/loop-carried/`setjmp`/address-taken/a `static`/a struct member.
  Compound loop tests (`getpass.c` `… && c != '\n'`; `t_raw.c` `(c &= 255) == 0177`) split via the
  negated stop test.
- **Hand-scan beyond the original 110-site grep** (the grep required the assignment to be the first
  token after `((`, so it missed member/subscript lvalues and non-leading folds): added `semaph.c`
  `(op_op[0].sem_op = value) == 0`, `bufargs.c` `argv[argc = 0]`, the `qid[i] = msgget(...)` folds
  (`ipc2/pollmsg.c`, `exercises/pollmsg2.c`), the `db.c` `db->field = …` folds (malloc/lseek/atoi/atol
  member assignments, 9 sites), `exercises/fmemopen.c` `ms->flags`/`ms->buf`, `termios/ttyname.c`
  `ddp->d_name = strdup(...)`, and the `cmptr == nullptr && (cmptr = malloc(...)) == nullptr` guard in
  `lib/sendfd.c`/`lib/recvfd.c`/`lib/Orecvfd.c`/`ipc2/sendfd2.c`/`ipc2/recvfd2.c` → nested `if`. Final
  re-grep: zero folds remain except the intentional `getcharbug.c`.
- **Hazards left as-is:** `stdio/getcharbug.c` kept nested + `char`-typed (the folded buggy idiom is the
  lesson; added an in-code note); `signals/read2.c` assignment split but the `int n` declaration left in
  place (setjmp function).
- **Execution:** `lib/` (33 sites) + the generator-source `advio/rot13a.c` + the hazard files done by
  hand; the 5 application-chapter groups (ipc2, printer, proc+intro, termios/signals/pty/sockets,
  scattered singletons) fanned out to 5 fresh general-purpose subagents (Edit-only; git/build/format
  forbidden), every diff reviewed on disk and gate-verified afterward — the pattern recorded in memory
  from the modernization fan-out.
- Baseline gate green before edits (371 targets compile, 8/8 `meson test`).

## Follow-on readability passes (requested 2026-10-16, William Emerison Six <billsix@gmail.com>)

After the flatten pass the maintainer asked to iterate further readability patterns on the same tree —
pull variable declarations closer to first use, then add scope-limiting braces — re-analyzing and
repeating until nothing more is found. Tracked here as successive passes; each is behavior-preserving
and gate-verified on its own. (Declare-at-use was already applied tree-wide in the modernization's S1,
so this pass targets residue the de-nesting newly exposed.)

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
