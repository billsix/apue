# Modernize the APUE example tree to modern C

**Status:** proposed — policy decided 2026-10-15; needs go-ahead to begin implementation
**Priority:** 7
**Difficulty:** 8
**Created:** 2026-10-15 (William Emerison Six <billsix@gmail.com>)

## Decisions (2026-10-15, William Emerison Six <billsix@gmail.com>)

Maintainer answered the Open questions:

1. **`-std` = c23.** Pin `c_std=c23`. *Unverified: the image's musl-clang must accept `-std=c23` — run
   the probe (below) before relying on it; fall back to c17 only if the probe fails.*
2. **Intent = (b) actually rewrite.** Modernize the example sources in modern-C style tree-wide — Tier
   C is now **in scope and applied**, not catalog-only. **This reverses the standing "match the
   textbook's style" convention** (`CLAUDE.md:63-67`, echoed in `README.md`): the tree stops being a
   byte-faithful companion to Stevens and becomes a modern-C rewrite of the same programs. Those docs
   must be updated as part of this work (see Doc consequences below).
3. **Scope = whole `apue.3e/` tree** (~220 programs + `lib/`, 223 `.c`). **`musl/` is explicitly OUT
   of scope** — it is vendored upstream libc built as a debuggable libc, not our code; never
   rewrite, reformat, or lint it (clang-tidy `-header-filter`/paths stay scoped to `apue.3e`).

This also resolves the blocker on `tasks/review-example-code-idiosyncrasies.md`, which is now
**superseded** and folded in as Tier B here.

### Doc consequences of decision 2 (required, part of the work)
- `CLAUDE.md:63-67` ("Match the textbook's style; this is reference code for learners") and the
  Conventions section — rewrite to state the new policy: modern-C style per
  `~/.claude/reference/c-coding-standard.md`, textbook fidelity no longer a goal.
- `README.md` / `CLAUDE.md` framing ("teaching / reference environment", "companion to the book") —
  reframe as a modernized rewrite.
- Output-equivalence to the *original programs' behavior* remains the correctness bar even though
  source fidelity is dropped (see "Output-equivalence" below).

## BLUF

Port the ~220 APUE-3e example programs and `lib/libapue.a` (223 `.c`, ~14 KLOC) from the current
`-std=c99` pin to a modern C standard (C17 recommended, C23 if the image's musl-clang supports it),
applying the shared C coding standard (`~/.claude/reference/c-coding-standard.md`). Per the 2026-10-15
decisions: pin **`-std=c23`**, rewrite the sources **tree-wide in modern-C style** (Tier C applied,
not just catalogued), **dropping the former textbook-fidelity convention**. "Done" = all three tiers
applied across the tree, the build green under a raised warning gate + clang-tidy, each program's
**runtime behavior unchanged** (output-equivalence), and `CLAUDE.md`/`README.md` reframed. **The hard
part is the scale and the equivalence proof**: 223 `.c` / ~14 KLOC, many needing a TTY/stdin and not
auto-tested — this is a multi-session step-task initiative, not one sitting.

## Context (cold-start orientation — read this first)

- **The code is upstream, not ours.** Sources are the apuebook.com examples (Stevens & Rago),
  `README.md:37-40` / `apue.3e/DISCLAIMER`. Local changes so far are only the header-inlining pass and
  the Meson port (both archived under `tasks/archive/2026/05/04/`). So most "quirks" are the
  textbook's own style, and **`CLAUDE.md:63-67` makes matching the textbook an explicit convention**
  ("Match the textbook's style; this is reference code for learners"). A blanket C23-style rewrite
  would defeat the point of the tree as a companion to the book — this tension is the whole task.
- **Current standard pin:** `apue.3e/meson.build:9` sets `c_std=c99`, with a comment explaining *why*
  c99 (fall-off-end-of-`main` returns 0, matches the `_POSIX_C_SOURCE 200809L` feature-test macros
  `apue.h` sets). Any `-std` bump must preserve both of those. `warning_level=1` today (`:10`).
- **Toolchain constraint:** C is compiled through the two-stage **musl-clang** wrapper driven by a
  Meson *native* file — read `tasks/reference/build-toolchain-musl-clang.md` before touching the
  build. Whether that clang accepts `-std=c23` is unverified (Open question 1).
- **The standard we apply:** `~/.claude/reference/c-coding-standard.md` splits into (a) what the
  toolchain enforces mechanically (format/lint/warnings/sanitizers — a green gate *is* the review)
  and (b) judgment calls no tool can check. The tiers below map onto that split.
- **`apue.h` conventions are kept** (`CLAUDE.md:11`): `include/apue.h` is a curated self-contained
  header; `.clang-format` is Google style, 80 cols, `SortIncludes: Never` (include order is curated).

## Relationship to existing tasks (do NOT duplicate)

- **`tasks/review-example-code-idiosyncrasies.md` — SUPERSEDED by this task (2026-10-15).** Its
  scope/intent blocker is resolved by the decisions above and its content (inconsistent `main`
  signatures, unused `argc`) is Tier B here. Archive it when this task's umbrella is scaffolded.
- **`tasks/add-sanitizer-gate.md` (proposed)** — the *dynamic* correctness gate (ASan/UBSan on
  executed paths). This task is the *static* side (standard pin, warnings, clang-tidy, source
  judgment calls). They are complementary: the sanitizer gate catches runtime UB; a `-Wconversion`
  sweep here catches it at compile time. Keep them separate tasks but run the sanitizer gate
  alongside Tier A so a modernized line is checked both ways.

## Proposed tiers (map to the standard's (a)/(b) split)

Ordered least-fidelity-risk first. **Only Tier A + Tier B are safe to apply to textbook code without
a policy decision; Tier C is catalog-first and likely stays opt-in per case.**

### Tier A — mechanical, toolchain-enforced, low fidelity risk
- [ ] **Pin a modern `-std`** in `apue.3e/meson.build:9` (c99 → `c_std=c17`, or `c23` if supported).
      C17 is a bug-fix release the reference doc calls "a safe, well-supported pin"; prefer it unless
      Open question 1 confirms c23. *Caveat: re-verify the two things the c99 comment protects —
      fall-off-end-of-`main`-returns-0 and the POSIX feature-test macros — still hold under the new
      pin before removing that comment.*
- [ ] **Raise the warning gate** toward the standard's list (`warning_level`, plus
      `-Wstrict-prototypes -Wold-style-definition -Wshadow -Wvla`, and — gated behind a count-first
      pass — `-Wconversion -Wsign-conversion`). *Caveat inline: textbook code will emit **many**
      conversion warnings; decide per Open question 2 whether to fix at source or scope the flag — do
      not blanket-`-Wno-` them.*
- [ ] **Add `.clang-tidy` + a `lint.sh`** (the repo has neither today) scoped to `apue.3e` sources
      (never the vendored `musl/`), running the `bugprone-`/`cert-`/`readability-`/`clang-analyzer-`
      families; diff and rebuild after any `-fix`.

### Tier B — clear defects, safe to fix even in textbook code
- [ ] **Non-prototype `int main()` → `int main(void)`** (7 sites, grep-confirmed 2026-10-15):
      `environ/hello1.c:3`, `exercises/pollmsg2.c:50`, `ipc2/pollmsg.c:43`, `signals/child.c:14`,
      `stdio/memstr.c:8`, `stdio/mkstemp.c:11`, `threads/barrier.c:74`. These are genuine K&R-style
      defects `-Wstrict-prototypes` flags (and under C23 empty parens mean `(void)` anyway). *This is
      exactly the idiosyncrasies task's core — see that task.*
- [ ] **Unchecked allocations / return values that are real bugs** (ERR33-C, MEM32-C) — ~28 files use
      `malloc`/`calloc`/`realloc`. *Caveat: the textbook deliberately omits some checks for brevity in
      the prose; distinguish a genuine latent bug from a pedagogically-intended omission, and when in
      doubt leave it and catalog it rather than diverging from the book.*

### Tier C — the (b) judgment calls — APPLIED tree-wide (per decision 2b)
These are the judgment calls in the standard. Decision 2b puts them **in scope**: rewrite the sources
to apply them. Still go class-by-class with output-equivalence after each class, not one giant diff:
- [ ] fixed-width `<stdint.h>` types where width matters; `size_t` for sizes/indices
- [ ] `const`-correctness on input pointers/bindings (DCL00-C)
- [ ] declare-at-first-use + initialize-at-declaration (the examples declare at top of `main`, book-style)
- [ ] `bool`/`true`/`false` over `int` flags; designated/`{ }` initializers
- [ ] `snprintf`/`memcpy`+length over `strcpy`/`strcat`/`strncpy` (STR31-C)
- [ ] real VLA audit — distinguish stack VLAs (a risk) from the globals/fixed arrays grep already
      found (`advio/nonblockw.c:11` is a global `char buf[500000]`, *not* a VLA; `advio/mandatory.c:14`
      is fixed `char buf[5]`). Keep `-Wvla` on to surface any genuine runtime-sized stack array.

## Suggested shape if go-ahead is given

Likely a **step-task umbrella** (this is 3+ sizeable sequential chunks): Step 1 = Tier A (build pin +
gates), Step 2 = Tier B defects (absorbing the idiosyncrasies task), Step 3 = Tier C decided class by
class. Don't scaffold the step-tasks until the Open questions are answered — the answers determine
whether Tier C exists at all.

**Output-equivalence is the safety net** (`~/.claude/reference/porting-with-an-output-oracle.md`): a
modernization must not change what a program prints/does. Build the parity check before editing — for
each wired-up `meson test` case, capture output before and diff after; many programs need a TTY/stdin
and aren't auto-tested (`CLAUDE.md:57-60`), so note which classes go unverified.

## Open questions

All three resolved 2026-10-15 — see the Decisions section at the top: (1) `-std=c23`, (2) rewrite
tree-wide (2b), (3) whole tree. One operational item remains before implementation:

1. **c23 probe (investigation, not a decision)** — confirm the image's musl-clang accepts `-std=c23`
   before the build pin relies on it; fall back to c17 if not. Cheap to run; does not need go-ahead.
