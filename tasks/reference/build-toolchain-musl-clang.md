# apue — build toolchain: musl-clang wrappers + the meson port

**Reference document** — how the APUE examples are built: compilation is routed through a two-stage
**musl-clang** wrapper, driven by a meson **native** file (not cross-compilation). This repo's first
reference doc. Not a task; update in place. Created 2026-08-27 (William Emerison Six <billsix@gmail.com>)
from a direct read (load-bearing anchors verified).

## The toolchain in one paragraph

meson compiles every C source with `/apue/musl/bldInstall/bin/musl-clang` (a wrapper around plain
`clang` that swaps glibc for a locally-built **musl** libc), links against musl statically-ish, and
produces the example binaries + a static `libapue.a`. It is a **native** build (host == target), not a
cross build — the wrapper just redirects the C library.

## Anchors (verified)

- **meson routes C through the wrapper:** `apue.3e/native-linux.ini:6`
  `c = '/apue/musl/bldInstall/bin/musl-clang'` — a **native file** (`:2-3` comment says so), no
  cross-compilation.
- **The wrapper is two stages** (`musl/tools/`): the **compile** wrapper (`musl-clang.in`) sets the musl
  sysroot (`-nostdinc --sysroot -isystem -B -fuse-ld -static-libgcc`); the **link** wrapper
  (`ld.musl-clang.in`) does the final `real-ld -nostdlib "$@" -lc -dynamic-linker <ldso>` with a
  `-l-user-start` / `-L-user-end` sentinel fence around the user's libs.
- **Template instantiation:** the `.in` wrappers are sed-substituted at build (`musl/Makefile` ~`:187`),
  with `@CC@` = plain `clang` (via `WRAPCC_CLANG=$(CC)` and the Dockerfile's `CC=clang`).
- **`libapue` is a STATIC archive** — `apue.3e/lib/meson.build:39` `static_library('apue', …)` →
  `libapue.a` (28 helper sources; a few deliberately excluded). (The repo `CLAUDE.md:25` already names it
  `libapue.a` — no mislabel.)
- **Build-graph specials:** sockets-before-printer coupling; `standards/` uses build-time codegen
  (`custom_target` + `scripts/gen_standards_c.sh`); `make build` does a `--wipe`-if-exists reconfigure;
  the musl-clang bin is put on `PATH` at runtime via `.extrabashrc`.

## Why this matters for the sanitizer gate (the actionable connection)

The link wrapper links **only musl `-lc` with `-nostdlib`** (`ld.musl-clang.in`). Fedora's ASan/UBSan
runtimes (`compiler-rt`) are built against **glibc**, so they **cannot link or load** under the
musl-clang wrapper. **This is the concrete reason `tasks/add-sanitizer-gate.md` must run its sanitizer
gate under a separate stock-`clang` + glibc meson config**, not through the musl-clang native file. That
task's feasibility question is answered by this toolchain design.

## Follow-on

Folded into the existing `tasks/add-sanitizer-gate.md` (this doc grounds its "run sanitizers under a
separate glibc-clang config" resolution) — no new task needed.

## Cross-links

- `tasks/add-sanitizer-gate.md` — the sanitizer gate this de-risks.
