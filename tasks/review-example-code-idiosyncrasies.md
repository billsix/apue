# Review the example code for idiosyncrasies (main signatures, unused argc, etc.)

**Status:** blocked
**Priority:** 6
**Difficulty:** 4
**Started:** 2026-08-27 (William Emerison Six <billsix@gmail.com>)
**Blocked on:** maintainer answers the Open questions below (scope: whole tree vs quirk-class sweep; and
intent: normalize vs catalog, given this is textbook-faithful Stevens code).
**Recheck:** the Open questions below are answered (maintainer-gated; `/recheck-blocked` surfaces it).

## Goal

Maintainer's idea, verbatim: *"Have a double check a lot of the code, it can have some weird
idiosyncrasies, like random void argc."*

Review the APUE example programs for idiosyncrasies and decide what (if anything) to normalize.

## Context (investigation 2026-08-27)

- Sources are **upstream Stevens code** (apuebook.com, `README:37-40`, `apue.3e/DISCLAIMER`), locally
  modified only by the header-inlining pass and the Meson port — so most quirks are the textbook's own
  style, and `CLAUDE.md:63-67` records that matching the textbook is itself a convention.
- The likely referent of "random void argc" — inconsistent `main` signatures across the tree:
  - `88×  int main(void)`, `34×  int main(int argc, char *argv[])`, and **`7×  int main()`** (empty
    parens — a non-prototype K&R-ish form): `environ/hello1.c:3`, `signals/child.c:14`,
    `exercises/pollmsg2.c:50`, `threads/barrier.c:74`, `stdio/mkstemp.c:11`, `stdio/memstr.c:8`,
    `ipc2/pollmsg.c:43`.
  - **Unused `argc`** (takes `int argc` but never reads it): `intro/testerror.c:8`, `ipc2/open.fe/main.c`,
    `ipc2/open/main.c`, `daemons/reread.c`, `daemons/reread2.c`.
  - No literal `void argc` token, no `(void)argc` casts, no K&R split-declaration `main(argc, argv)`
    were found — so "random void argc" is most plausibly this cluster loosely described.
- No existing task covers a style/idiosyncrasy review. `tasks/add-sanitizer-gate.md` is a *dynamic*
  correctness gate (UB/memory on executed paths), not a signature/style review — keep them separate.
- `.clang-format` exists (Google style, `SortIncludes: Never`). `TODO.org`'s one open style sub-item is
  narrow (`[ ] maybe clang-format the includes to sort them`).

## Plan (draft — after questions answered)

- [ ] Sweep the three grep-able quirk classes tree-wide (empty-paren `int main()`, unused `argc`,
      `main(void)` inconsistency) and catalog them.
- [ ] Per Q2 policy: fix only clear defects (e.g. non-prototype `int main()` → `int main(void)`), leave
      textbook-faithful style alone.

## Open questions

1. **Scope** — cover the *whole* tree (~220 programs + `lib/`), or a representative sweep of the quirk
   classes found above? *(Recommend: scoped sweep of those three grep-able classes tree-wide.)*
2. **Intent** — *normalize* the quirks (consistent signatures, drop unused `argc`) or just *catalog*
   them, given this is upstream Stevens teaching code where textbook fidelity is a stated convention?
   *(Recommend: catalog + fix only clear defects like non-prototype `int main()` → `int main(void)`;
   leave textbook-faithful style; record the policy in this task.)*
