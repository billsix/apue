# Add an ASan + UBSan build gate to apue

**Status:** proposed — needs go-ahead
**Created:** 2026-06-16

## Goal

Extend apue's image build so that, like the existing `meson test` suite gate, it
*also* compiles the apue example tree under sanitizers and runs the test suite
under each, **failing the image if any undefined behavior or memory error fires**.
This catches integer-UB and memory-safety landmines in the textbook example code
before they ship.

Reference template: spimulator just landed exactly this gate. See its archived
sweep doc — `/billopt/spimulator/tasks/archive/2026/06/16/ubsan-sweep.md` — for a
full primer on what ASan/UBSan are, the diagnostic-vs-trap distinction, and the
integer-UB rationale (signed overflow, bad shifts). The wiring to copy lives in
`/billopt/spimulator/Dockerfile` (lines ~141–164, gated on `RUN_SANITIZERS`) and
`/billopt/spimulator/Makefile` (`RUN_SANITIZERS ?= 1`, threaded through as a
`--build-arg`). Short version of the technique:

- **UBSan in trap mode** — `-fsanitize=undefined -fsanitize-trap=undefined`. No
  runtime to link; any UB kills the process with SIGILL. This is the reliable
  **pass/fail gate**. (Diagnostic `-Db_sanitize=undefined` under-reports and needs
  a runtime `.so` — keep it as a locating aid only, not the gate.)
- **ASan** — `-Db_sanitize=address` (meson) for buffer overflow / use-after-free /
  double-free / leaks.
- If a program leaks at exit by design, default LeakSanitizer off with a weak
  `const char* __asan_default_options(void){ return "detect_leaks=0"; }` compiled
  only into ASan builds (spim does this in `spim.c`).

## ⚠️ KEY FEASIBILITY QUESTION — musl-clang vs. the sanitizer runtimes

**This is the make-or-break issue and must be resolved first.** apue's Linux
toolchain is **musl-clang**, a wrapper built from vendored musl
(`native-linux.ini` → `/apue/musl/bldInstall/bin/musl-clang`). Reading the two
wrapper scripts (`musl/tools/musl-clang.in`, `musl/tools/ld.musl-clang.in`) shows
why sanitizers are problematic on this target:

- The compile wrapper passes `-nostdinc --sysroot <musl> -isystem <musl-inc>` —
  it builds against the **musl** headers/sysroot, not glibc.
- The link wrapper (`ld.musl-clang`) runs the real `ld` with **`-nostdlib`** and
  links essentially only **`-lc`** (musl) plus user `-l`s inside the
  `-l-user-start/-end` fence. It deliberately strips the normal driver link line.

Fedora's `compiler-rt` sanitizer runtimes (`libclang_rt.asan-*.a/.so`,
`libclang_rt.ubsan_standalone-*`) are **built against glibc** and are *not* present
in the musl sysroot, and the musl-clang link wrapper won't pull them in anyway. So:

- **ASan under musl-clang: expected NOT to work** without porting/building a
  musl-targeted compiler-rt — the asan runtime won't link/load against musl.
- **Diagnostic UBSan under musl-clang: expected NOT to work** for the same reason
  (it needs `ubsan_standalone`).
- **UBSan *trap* mode under musl-clang: plausibly the ONLY thing that works on the
  musl target**, because trap mode emits `ud2` inline and links **no runtime** at
  all. This is the one lane worth attempting against musl-clang directly.

**Proposed resolution (decide during implementation):** run the sanitizer gate as
a **separate Meson configuration using stock Fedora `clang` + glibc** (i.e. *not*
the `native-linux.ini` musl path — configure with `CC=clang` and no native file,
exactly like spim does), reusing apue's own `meson.build`. The example sources are
ordinary POSIX C; they compile fine under glibc-clang, and glibc-clang has the
full compiler-rt runtimes Fedora already ships (the `clang`/`llvm` packages are
already in the Dockerfile). The musl build remains the project's primary
toolchain and is untouched; the sanitizer build is an *additional* glibc-clang
configuration whose only purpose is the correctness gate.

- First implementation step is a **feasibility spike**: in the container, try
  `CC=clang meson setup /tmp/san-asan /apue/apue.3e -Db_sanitize=address` (no
  native file) + `meson compile` + `meson test`, and the same for UBSan-trap. If
  glibc-clang builds and tests the tree cleanly, the gate is practical via that
  path. Also probe whether UBSan-**trap** works through `musl-clang` itself (no
  runtime needed) — if it does, a musl-native trap lane is a bonus.
- **Honest fallback:** if neither glibc-clang nor musl-trap can build+test the
  tree, the gate is **not practical for apue right now** and the task should stop
  with that recorded, rather than shipping a half-working lane.

## Proposed build-system changes

Mirror spimulator's structure:

1. **Makefile** — add `RUN_SANITIZERS ?= 1` near the top and thread it through the
   `image` target as `--build-arg RUN_SANITIZERS=$(RUN_SANITIZERS)`. `make image
   RUN_SANITIZERS=0` skips the gate. Document the var with a `#` comment.
2. **Dockerfile** — add `ARG RUN_SANITIZERS=0` (Dockerfile default 0, Makefile
   default 1, per the family convention). After the existing `meson compile` /
   `meson test` of the musl build, add a `RUN if [ "$RUN_SANITIZERS" = "1" ]; then
   … fi` block that, **using stock `clang`/glibc (no `native-linux.ini`)**:
   - configures `/tmp/san-ubsan` with
     `-Dc_args='-fsanitize=undefined -fsanitize-trap=undefined'`
     `-Dc_link_args='-fsanitize=undefined -fsanitize-trap=undefined'`,
     `meson compile`, then `meson test --print-errorlogs`;
   - configures `/tmp/san-asan` with `-Db_sanitize=address`, `meson compile`,
     `meson test --print-errorlogs`;
   - `rm -rf` both temp builddirs.
   `set -e` so any UB/memory error fails the image.
3. **Scope — sanitize apue's own code only, not vendored musl.** The sanitizer
   configs build against glibc, so vendored musl isn't in the picture for that
   lane (this sidesteps spim's "don't sanitize the `-nostdlib` demos" problem).
   Pass sanitizer flags via `-Dc_args`/`-Db_sanitize` on the apue tree's own
   configure; do **not** instrument the musl build under `native-linux.ini`.
4. **ASan leak hook (if needed):** apue's example programs are short-lived and
   many intentionally don't free before exit. If ASan's bundled LeakSanitizer
   produces noise at exit, add the weak `__asan_default_options` returning
   `"detect_leaks=0"` to a shared TU compiled into every example — most naturally
   `lib/` (linked into `libapue.a`, which every example links) — guarded so it's
   present only in ASan builds. Decide based on the spike: if the suite is clean
   with leak detection on, leave it on (leaks are real bugs worth catching).

## What to gate on

The existing `meson test` suite — the same cases the normal build already gates
on. **Note the coverage caveat honestly:** apue's suite is *thin*. Only three
chapter dirs wire up `test()` at all — `intro/` (hello, uidgid, ls1, testerror),
`environ/` (hello1, getrlimit), `threads/` (threadid, exitstatus): **8 tests
total**, out of ~41 built executables across ~25 chapter dirs. Sanitizers are
dynamic — they only catch bugs on code paths that actually run — so this gate's
reach is limited by that thin suite. It's still worth having (it catches
regressions in the covered paths and any UB in `libapue.a`/startup that every test
exercises), but the doc/commit should be explicit that **broadening the gate's
value means broadening the test suite**, which is out of scope here.

## In-container-only constraint

All build/verify work runs **inside the apue container** (`make build` / a shell),
per the standing arrangement — no host installs. The glibc-clang sanitizer path
needs no new packages (`clang`, `llvm`, `gcc` are already in the Dockerfile;
Fedora's `clang`/`compiler-rt` ship the asan/ubsan runtimes). If the spike turns
out to need an explicit `compiler-rt`/`libclang_rt` dnf package, that's a tracked,
temporary Dockerfile addition during the spike — but verify it's actually missing
first.

## Acceptance criteria

- Feasibility verdict recorded in this doc **before** any gate is wired: does
  glibc-clang build+test the apue tree under ASan and UBSan-trap in the container?
  (And, as a bonus, does musl-clang UBSan-trap work?)
- If practical: `make image` (default `RUN_SANITIZERS=1`) builds apue a second
  time under **UBSan-trap** and under **ASan**, runs `meson test` under each, and
  **fails the image** on any UB or memory error. `make image RUN_SANITIZERS=0`
  builds without the gate.
- Any UB/memory sites surfaced are fixed smallest-diff-first (compute in unsigned
  where wrap is intended; cast at the boundary), preserving behavior — matching
  the spim sweep's methodology. Each fix noted (file:line, what + why) here.
- ASan-leak decision recorded (detect_leaks left on, or `__asan_default_options`
  hook added and where).
- Dockerfile `ARG` default 0 / Makefile `RUN_SANITIZERS` default 1, `--build-arg`
  threaded, `make image RUN_SANITIZERS=0` documented — consistent with spim and
  the family Makefile contract.
- Coverage caveat (8 tests / ~41 exes) called out in the commit/PR so the gate's
  limited reach isn't oversold.

## Notes

- This is a *shipped-build/CI policy choice* (default-on gate on every image
  build), the same call Bill made for spim — flag it explicitly when proposing the
  default.
- Don't silence sites with `__attribute__((no_sanitize(...)))` unless genuinely
  UB-free-by-construction and documented; prefer real fixes.
- Move this doc to `tasks/archive/<YYYY>/<MM>/<DD>/` when complete.
