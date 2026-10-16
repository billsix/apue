# apue

A modernized build of the example code from *Advanced Programming in the UNIX
Environment, 3rd edition* (W. Richard Stevens & Stephen A. Rago). The ~220
example programs from apuebook.com live under `apue.3e/`; this fork re-tooled
them for a clean, debuggable, containerized build and **modernized the
sources to modern C (C23)** (`tasks/archive/2026/10/16/modernize-to-modern-c.md`). It's a teaching /
reference environment for UNIX systems programming — modern-C reference, no
longer a byte-faithful transcription of the book.

## Status

- **Language/standard:** C (`-std=c23`). The examples have been **modernized to modern C** per
  `github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md` (declare-at-use, `const`, `bool`, `nullptr`, `snprintf`,
  designated initializers, `size_t` indices) — see `tasks/archive/2026/10/16/modernize-to-modern-c.md`.
  The tree compiles **warning-clean under `-Werror`** (`warning_level=2` + the extra flags;
  `werror=true`, overridable with `-Dwerror=false`). The curated `apue.h` surface is kept;
  textbook *source* fidelity is no longer a goal (the behavior is preserved).
- **Build:** Meson + Ninja (the original recursive Make was replaced). Out-of-tree,
  in `apue.3e/build/`.
- **Toolchains:** Linux via **musl-clang** (built from source in the image,
  selected through `native-linux.ini`); macOS via system clang. FreeBSD/Solaris
  were dropped — see `tasks/archive/2026/05/04/02-meson-build.md`.
- **Header inlining done:** each `.c` now `#include`s exactly the system headers
  it uses (with `/* for … */` comments) instead of the old grab-bag `apue2.h`;
  `apue.h` is last in the include list. See
  `tasks/archive/2026/05/04/01-header-inlining.md`.

## Layout

- `apue.3e/` — the textbook example tree. `include/apue.h` (shared declarations);
  `lib/` → `libapue.a` (28 helper sources: error/daemon/pty/lock/IPC); ~19 chapter
  dirs (`intro/`, `fileio/`, `proc/`, `signals/`, `threads/`, `sockets/`, `ipc1/`,
  `ipc2/`, …), each with its own `meson.build`. `native-linux.ini` /
  `native-macos.ini` are the Meson native files.
- `musl/` — vendored musl + its own Make; built into the image with `clang -g -O0`
  and installed to `/apue/musl/bldInstall` (a debuggable libc).
- `entrypoint/` — `shell.sh` (`set -e`, cd `/apue`, `exec bash "$@"` — powers `make shell` and its batch twin `make shell-exec`) and `dotfiles/`
  (`.extrabashrc` puts musl-clang on `PATH`; `.lldbinit` disables ASLR + shows
  breakpoint context).
- `tasks/` — in-flight work; `tasks/archive/<YYYY>/<MM>/<DD>/` holds completed
  work, including the now-finished modernization design docs
  (`2026/05/04/01-header-inlining.md`, `02-meson-build.md`,
  `03-musl-clang-warnings.md`).
- `Dockerfile`, `Makefile`, `TODO.org`.

## Build / container workflow

Fedora-44 + podman, the usual family template. `make` targets:

- `make help` *(default)* — list the documented targets.
- `make image` — build the image (`apue`); compiles musl-clang from source.
- `make shell` — ephemeral container, mounts `apue.3e/`, drops to bash.
  Adds `--cap-add=SYS_ADMIN --security-opt seccomp=unconfined` so lldb/gdb can ptrace.
- `make build` — inside the container: `meson setup build --native-file
  native-linux.ini && meson compile -C build && meson test -C build`.
- `make format` — clang-format all `apue.3e` C.

There is **no `ENTRYPOINT`** in the image — every target passes
`--entrypoint /bin/bash` (a prior stale entrypoint was removed; see the Dockerfile
comment).

## Tests

`meson test` runs the per-chapter `test()` cases that are wired up (e.g. `intro/`
smoke-tests `hello`, `uidgid`, `ls1`, `testerror`). Coverage is partial — many
programs need a TTY/stdin and aren't auto-tested.

## Conventions

- Clang-format: Google style, 80 cols, pointer-right, **`SortIncludes: Never`**
  (include order is curated — see
  `tasks/archive/2026/05/04/01-header-inlining.md`).
- **Modernize to modern C (C23)**, not textbook-faithful transcription: apply the shared standard
  `github.com/billsix/runClaudeInContainer:tasks/reference/c-coding-standard.md` (fixed-width types where width matters, `const`,
  `nullptr`, `bool`, `[[maybe_unused]]`, declare-at-first-use, `snprintf` over `strcpy`/`strcat`,
  designated initializers). **Preserve each program's runtime behavior** — the wired `meson test`
  cases and a green compile are the oracle. Keep each file's includes explicit and commented.
- **Teaching style — flatten nested expressions** for C/Unix newcomers: prefer plain steps over an
  assignment folded into its test (split `(c = getc(stdin)) != EOF` into an assignment and a plain
  `c != EOF` compare; give a deeply nested sub-expression a named intermediate). A deliberate
  pedagogical choice that **overrides the shared standard's expression-density idioms for the
  `apue.3e/` example tree**; behavior-preserving. Pattern + rationale:
  `tasks/reference/teaching-flatten-expressions.md`; tree-wide application (not yet done):
  `tasks/flatten-nested-expressions.md` (proposed).
- Lint: `apue.3e/.clang-tidy` (scoped to the tree; `musl/` is never linted) + `apue.3e/tools/lint.sh`
  (`tools/lint.sh [--fix] [dir...]`). Advisory; the modernization sweep fixes findings at the source.
- Generated sources: `advio/rot13c2.c` is generated from `rot13c2.c.in` + `scripts/gen_rot13c2.sh`
  (splices `translate()` from `rot13a.c` via `fixup.awk`) — edit the template/generator, never the
  generated file; the awk matcher is sensitive to `rot13a.c`'s formatting.

## Tasks

Per the global convention, active work goes in `tasks/`; completed work is moved
to `tasks/archive/<YYYY>/<MM>/<DD>/`. The modernization design docs (header
inlining, Meson port, musl-clang warning suppression — all done) live under
`tasks/archive/2026/05/04/`.

**Reference docs:** `tasks/reference/build-toolchain-musl-clang.md` (added 2026-08-27) —
how the examples build through the two-stage musl-clang wrapper + the meson native file;
read it before touching the build (it explains why the sanitizer gate needs a separate
glibc-clang config).
