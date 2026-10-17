# Modernization parity with the gltron C sweep

**Status:** proposed — needs go-ahead
**Priority:** 6
**Difficulty:** 4
**Created:** 2026-10-17 (William Emerison Six <billsix@gmail.com>)

## BLUF

apue was modernized to C23 on 2026-10-16 (`tasks/archive/2026/10/16/modernize-to-modern-c.md`):
meson, warning-clean under `-Werror`, `nullptr`, pointee-`const`, `snprintf`. A sibling project
(gltron) ran its own behavior-preserving C sweep on 2026-10-17
(`github.com/billsix/gltron-mirror`, `tasks/archive/2026/10/17/c-modernization-sweep.md`) and went a
few steps further. This task is the **gap list** — the specific things gltron did that apue still
lacks — to bring apue to parity. All changes are behavior-preserving; apue is a modern-C reference,
so clarity/safety only, no churn for its own sake.

## Gaps (surveyed 2026-10-17; each is HAS/GAP vs gltron)

1. **`const T* const` (const pointer to const) — GAP (0 uses tree-wide).** apue has 116 plain
   `const char*` (pointee-const done) but **zero** double-const. Adopt `const char* const p` for
   pointer locals/params that are never reassigned and point to const (gltron did, e.g. its
   `directory-macos.c`). This is the headline item and the direct subject of the maintainer's
   request. **Also fold the shared C standard's new "double-const" guidance** (added to
   `c-coding-standard.md` by the gltron work) once it lands.
2. **Warning flags — GAP.** apue's `apue.3e/meson.build` has `warning_level=2`, `werror=true`,
   `-Wshadow -Wvla -Wstrict-prototypes -Wold-style-definition -Wwrite-strings -Wformat=2`. Missing
   vs gltron: **`-Wpedantic`**, **`-Wsign-conversion`** (explicitly deferred at `meson.build:26`
   "held back until a count-first pass" — gltron turned it on with boundary casts), and clang-only
   **`-Wshorten-64-to-32` / `-Wimplicit-int-conversion`**. Do the count-first pass, then enable.
3. **Dual-compiler build — GAP.** apue builds **clang-only** (musl-clang on Linux, system clang on
   macOS); gcc is installed in the Dockerfile but has no build lane, so apue is not proven
   gcc-`-Werror`-clean. gltron builds clean under both. Add a gcc lane (or document why not —
   note the musl-clang constraint in `tasks/reference/build-toolchain-musl-clang.md`).
4. **Sanitizer gate — GAP, already tracked.** No UBSan-trap + ASan gate, no `tools/sanitizer-gate.sh`.
   This is **`tasks/add-sanitizer-gate.md`** (proposed) — do it there, not here; it documents the
   real musl-clang feasibility blocker (compiler-rt vs glibc) that gltron did not face. This task
   just cross-references it.
5. **C23 attributes — GAP (partial).** apue has `nullptr` (364), `[[maybe_unused]]` (50),
   `[[noreturn]]` (7), zero `__attribute__` — strong. Missing: **`[[nodiscard]]`** on allocators /
   result helpers in `lib/`, and **`[[fallthrough]]`** on intentional switch fall-throughs.
6. **Buffers — GAP.** 4 real `sprintf` calls remain (`db/db.c:598`, `printer/printd.c:795`, `:797`,
   `:802`) → `snprintf`. 22 `strcpy`/`strcat` worth a buffer-safety look. ~15 of 57 `snprintf` bounds
   use a magic `MAXLINE - 1` rather than `sizeof(buf)` (gltron's "name the buffer size once" idiom —
   also now in the shared C standard). Verify the ~63 malloc/calloc/realloc sites for NULL-check +
   owner-contract comments.
7. **Advisory lint — GAP.** `apue.3e/tools/lint.sh` runs **clang-tidy only**; add a **cppcheck** pass
   (`--enable=warning,style,performance,portability`), matching gltron's `tools/lint.sh`. apue's
   `.clang-tidy` is already well scoped (`HeaderFilterRegex`) — a good model gltron could borrow.

## Method & verification

Follow the gltron sweep's playbook (the archived doc + the shared `c-coding-standard.md` /
`c-modernization-sweep.md` reference docs in `github.com/billsix/runClaudeInContainer`): class-by-class,
behavior-preserving, boundary casts not retypes for sign-conversion, fresh per-directory Edit-only
subagents for the mechanical bulk, verify each with the raised-warning build. apue's existing gate
(`werror=true`) plus the new flags is the primary net; the sanitizer gate (gap 4) is the runtime net
once it lands. apue has no general test suite, so state per-change what went unverified.

## Note

This is a gap/parity task, not a rewrite — apue is already modern C. Most items are small. Order:
(2)+(3) warning/compiler parity first (they define "done"), then (1) const-const and (5)/(6) idioms
under the raised gate, (7) lint anytime; (4) is its own tracked task.
