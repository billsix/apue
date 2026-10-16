# Modernize the APUE example tree to modern C

**Status:** **COMPLETE** (2026-10-16). The ~220 APUE-3e example programs and `lib/libapue.a` under
`apue.3e/` were rewritten from `-std=c99` to **`-std=c23`** modern-C style per
`github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md`, with each program's runtime behavior preserved. The tree
compiles **warning-clean** at `warning_level=2` + targeted flags + **`-Werror`**, and the 8 wired
`meson test` cases pass. `musl/` was out of scope throughout.
**Priority:** 7 **Difficulty:** 8
**Created:** 2026-10-15 (William Emerison Six <billsix@gmail.com>) · **Completed:** 2026-10-16

## BLUF

Ported the ~220 example programs + `lib/` (223 `.c`, ~14 KLOC) from `c99` to `c23`, applying the
shared C coding standard tree-wide and **dropping the former textbook-source-fidelity convention**
(behavior, not source text, is the contract). "Done" = the standard's applicable items applied across
the tree, the build green under the raised warning gate + `-Werror` + clang-tidy, and each program's
**runtime behavior unchanged** (output-equivalence oracle). Delivered as two commits on top of the
pre-existing history: **phase 1** (build/lint backbone + mechanical changes) and **phase 2**
(per-chapter judgment-call rewrite). Docs (`CLAUDE.md`, `README.md`) reframed to the modern-C policy.

## Decisions (2026-10-15, William Emerison Six <billsix@gmail.com>)

1. **`-std` = c23.** The image's musl-clang is `clang 22.1.4`, which accepts `-std=c23` and compiles
   `nullptr`/`true`/`false`/attributes clean. Fall-off-end-of-`main` still returns 0 (as c99 did,
   which `hello1.c` relies on) and the POSIX.1-2008 feature-test macros `apue.h` sets are unaffected.
2. **Intent = actually rewrite (not catalog).** Modernize the sources tree-wide in modern-C style.
   **This reversed the standing "match the textbook's style" convention** — the tree is now a modern-C
   rewrite of the same programs, no longer a byte-faithful companion to Stevens. `CLAUDE.md`/`README.md`
   were updated to state the new policy.
3. **Scope = whole `apue.3e/` tree** (~220 programs + `lib/`). **`musl/` is OUT of scope** — vendored
   upstream libc, never rewritten/reformatted/linted (clang-tidy paths stay scoped to `apue.3e`).

The correctness bar remained **output-equivalence to the original programs' behavior**
(`github.com/billsix/runClaudeInContainer:tasks/reference/porting-with-an-output-oracle.md`), even though source fidelity was dropped.

## Correctness oracle (used on every commit)

- `meson compile` builds all 371 targets clean; now also under **`-Werror`**.
- The 8 wired `meson test` cases pass (`intro_hello`, `intro_uidgid`, `intro_ls1`, `intro_testerror`,
  `environ_hello1`, `environ_getrlimit`, `threads_exitstatus`, `threads_threadid`). Coverage is partial
  — most programs need a TTY/stdin and aren't auto-tested — so untested chapters were verified by green
  compile + clang-tidy-clean + keeping each transform mechanical/behavior-preserving.
- Gate run without the nested `: image` rebuild: the build's own
  `podman run … apue -c 'cd /apue/apue.3e && meson …'` line against the existing image.

## What was done

**Phase 1 — backbone + mechanical/clearly-correct changes** (commit: "phase 1"):
- Pinned `c_std=c23`; raised the gate to `warning_level=2` + `-Wshadow -Wvla -Wstrict-prototypes
  -Wold-style-definition -Wwrite-strings -Wformat=2` (each `cc.has_argument`-guarded).
- Added scoped `apue.3e/.clang-tidy` + portable `apue.3e/tools/lint.sh` (`musl/` never linted;
  `cert-err33-c` and the Annex-K-recommending `insecureAPI` check disabled — N1967).
- Tree-wide `NULL`→`nullptr`; all 16 exec sentinels normalized to `(char *)0`; dropped dead
  `<stddef.h>` includes. `[[maybe_unused]]` on 48 intentional unused params; `[[noreturn]]` replaced
  the last `__attribute__` in `apue.h`; `int main()`→`int main(void)` at 7 sites.
- Five gate-caught real bugs, incl. `serv_accept`'s `malloc(sizeof(un.sun_path + 1))` (8 bytes, not
  109 — a buffer overflow) and the rot13c2 generator producing an empty `translate()` because a
  reformat had broken `fixup.awk`'s matcher.

**Phase 2 — per-chapter judgment-call rewrite** (commit: "phase 2"), all 21 chapters, gate-green:
- **S1 — declare-at-first-use + initialize + one-decl-per-line + `const`.** Loop indices into the
  `for`; `const` on once-assigned values and read-only pointees/params (forward prototype updated when
  a param type changed); NOT on resource-handle pointers (FILE*/DIR*).
- **S2 — `bool`/`true`/`false`** for genuine truth-value flags (pty, printer, cliconn). Bitmask flags,
  mode selectors (`db_store` flag), `setsockopt` optval, and `sig_atomic_t` handler flags kept `int`.
- **S3 — eliminate `-Wsign-compare`** (tree now compiles with zero warnings). `size_t`/`ssize_t` for
  sizes and indices; db.c read/write-result comparisons cast the length to `(ssize_t)`; the two musl
  `CMSG_NXTHDR` macro sites (not our code) wrapped in a narrow, commented `#pragma` suppression.
- **S4 — `snprintf`/`memcpy` over `sprintf`/`strcpy`+`strcat`/non-terminating `strncpy`** where the
  destination size is known. Hardened a previously unguarded `sun_path` copy in `nspipe.c`; bounded
  the trailing-newline append in `error.c`/`errorlog.c`. Left copies into exactly-sized `malloc`
  buffers, bare `char *` params with no size in scope, and `print.c`'s `strncpy(req.usernm,…)` whose
  tail zero-fill is serialized over the wire (snprintf would leak stack).
- **S5 — designated / `{ }` initializers over `memset`-zero** (`addrinfo hint` designated inits;
  `sockaddr_un`/`mymesg` → `= {0}`). In-loop re-zeroing and non-zero fills left as-is.
- **S6 (optional tier) — reviewed and declined.** free-once-null: all 32 `free()` sites are terminal.
  `arr[static N]`/`[[nodiscard]]`: would churn the curated apue.h surface for sanitizer-only benefit.
  No compelling `static_assert`/`auto`/`typeof` site; no intentional switch fallthrough.

**Docs + gate:** reframed `CLAUDE.md`/`README.md` to the modern-C (C23) policy; turned on `-Werror`
(meson `werror=true`, overridable) now that the tree is warning-clean.

**Hazards deliberately left intact** (behavior/teaching value): `setjmp`/`longjmp` live locals,
loop-carried accumulators, address-taken/filled locals, deliberate teaching bugs (`getcharbug` char/EOF,
`scope.c`, `testjmp.c`, `mkstemp` bad_template, `badexit2` dangling return, `wait1` SIGFPE demo,
`vfork3` lesson, intentional data-race demos), and generated-file sources (`rot13a.c`/`rot13c2.c.in`).
KEEP POSIX types (`pid_t`/`off_t`/`ssize_t`/`mode_t`) — correct, not a plain-`int` smell.

## Standard-coverage audit (`github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md`)

| Standard item | Status | Note |
| --- | --- | --- |
| declare-at-first-use + initialize | **done** | S1, all 21 chapters |
| one declaration per line | **done** | split tree-wide in S1 |
| `const` by default | **done** | S1: const on once-assigned values + read-only pointees/params |
| fixed-width `<stdint.h>` types | **n/a** | no wire formats; POSIX types correctly kept |
| `size_t` for indices | **done** | S3: index/length types fixed; zero sign-compare remain |
| `bool`/`true`/`false` | **done** | S2: pty/printer/cliconn flags → bool (bitmasks/modes kept int) |
| integer-conversion (sign-compare) | **done** | S3: zero `-Wsign-compare`; CMSG-macro ones suppressed narrowly |
| one owner / one free, check allocations | **done** | textbook checks present; serv_accept bug fixed |
| free-once-then-null | **n/a (reviewed)** | S6: all 32 free() sites terminal (free-then-return/exit) |
| `-Wvla` on; no stack VLAs | **done** | gate on; none exist (globals/fixed arrays only) |
| `arr[static N]`, pass length | **declined (reviewed)** | S6: would change the curated apue.h surface; sanitizer-only benefit |
| `snprintf` over `strcpy`/`strcat`/`sprintf` | **done** | S4: where size known; exactly-sized malloc + wire-format zero-fill kept |
| no Annex K / no `gets` | **done** | none present; the `_s`-recommending check is disabled |
| one error strategy | **done** | err_/log_ family via apue.h, consistent |
| designated / `{ }` initializers | **done** | S5: addrinfo → designated init; sockaddr/mymesg → {0} over memset |
| real construct over macro / `_Generic` | **n/a** | apue.h min/max/lock macros are the documented surface |
| `static_assert` | **declined (reviewed)** | S6: no compelling compile-time invariant in the examples |
| `nullptr` | **done** | tree-wide (phase 1) |
| `[[maybe_unused]]` / `[[noreturn]]` | **done** | phase 1; `[[nodiscard]]` declined (S6); no intentional fallthrough |
| `auto`/`typeof`/`constexpr` | **declined (reviewed)** | S6: no compelling site; "use sparingly" |
| type-pun via memcpy / strict aliasing / UB | **done (gate)** | gate-caught UB fixed; runtime UB → `tasks/add-sanitizer-gate.md` |
| self-contained headers; static internal | **done** | apue.h self-contained; lib API is public by design |
| snake_case naming; present-tense comments | **done** | textbook naming kept; comments current |
| concurrency (pthreads, no data race) | **n/a** | threads examples use pthreads correctly |

## History & lessons

- 2026-10-15 — Phase 1 landed (backbone + mechanical changes), squashed to one commit; 8/8 tests,
  23 benign `-Wsign-compare` left for the per-chapter work.
- 2026-10-16 — Phase 2 (S1–S5) across all 21 chapters. The first 13 chapters were done by hand; the
  rest were fanned out to subagents. **Process lesson:** `subagent_type: "fork"` workers inherit the
  parent's coordinator context and two of them tried to manage/stop the others — one ran a broad
  `git checkout` that silently reverted five finished chapters' unstaged edits (unrecoverable from
  git). Recovery: redid those with **fresh general-purpose subagents, git/build/format forbidden,
  verifying on-disk state and staging each immediately**. That pattern then carried the S4 (snprintf)
  fan-out cleanly. (Recorded in memory: use fresh general-purpose subagents, not forks, for mechanical
  fan-out over a shared tree.) S2, S3, S4, S5 then applied; S6 reviewed and declined; `-Werror`
  turned on (Step 4b). History squashed to phase-1 + phase-2; docs reframed.

## Related tasks & artifacts

- **`tasks/archive/2026/10/16/review-example-code-idiosyncrasies.md`** — SUPERSEDED by this task (2026-10-15); its content
  (inconsistent `main` signatures, unused `argc`) was folded in and is done.
- **`tasks/add-sanitizer-gate.md` (proposed)** — the complementary *dynamic* correctness gate
  (ASan/UBSan on executed paths); this task was the *static* side.
- **`tasks/reference/build-toolchain-musl-clang.md`** — read before touching the build.
- Reusable codemods from phase 1 live under `tasks/adhoc/modernize-to-modern-c/` (`nullptr.sh`,
  `maybe_unused.py`).
- Generated source: `advio/rot13c2.c` is generated from `rot13c2.c.in` + `scripts/gen_rot13c2.sh`
  (splices `translate()` from `rot13a.c` via `fixup.awk`) — edit the template/generator, never the
  generated file.
