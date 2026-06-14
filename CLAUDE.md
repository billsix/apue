# apue

A modernized build of the example code from *Advanced Programming in the UNIX
Environment, 3rd edition* (W. Richard Stevens & Stephen A. Rago). The ~220
example programs from apuebook.com live under `apue.3e/`; this fork re-tooled
them for a clean, debuggable, containerized build. It's a teaching / reference
environment for UNIX systems programming.

## Status

- **Language/standard:** C (`-std=c99`), the textbook's `apue.h` conventions kept.
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
- `entrypoint/` — `shell.sh` (cd `/apue`, exec bash) and `dotfiles/`
  (`.extrabashrc` puts musl-clang on `PATH`; `.lldbinit` disables ASLR + shows
  breakpoint context).
- `tasks/` — in-flight work; `tasks/archive/<YYYY>/<MM>/<DD>/` holds completed
  work, including the now-finished modernization design docs
  (`2026/05/04/01-header-inlining.md`, `02-meson-build.md`,
  `03-musl-clang-warnings.md`).
- `Dockerfile`, `Makefile`, `TODO.org`.

## Build / container workflow

Fedora-44 + podman, the usual family template. `make` targets:

- `make image` — build the image (`apue`); compiles musl-clang from source.
- `make shell` *(default)* — ephemeral container, mounts `apue.3e/`, drops to bash.
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
- Match the textbook's style; this is reference code for learners. Keep each
  file's includes explicit and commented.

## Tasks

Per the global convention, active work goes in `tasks/`; completed work is moved
to `tasks/archive/<YYYY>/<MM>/<DD>/`. The modernization design docs (header
inlining, Meson port, musl-clang warning suppression — all done) live under
`tasks/archive/2026/05/04/`.
