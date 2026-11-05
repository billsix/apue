# Upgrade the container base from Fedora 44 to Fedora 45 (done)

Done 2026-11-05 (William Emerison Six <billsix@gmail.com>). Part of a fleet-wide
Fedora 44 → 45 bump.

## What changed

- `Dockerfile`: `registry.fedoraproject.org/fedora:44` → `fedora:45`.

No source changes were needed. `make image` is green nested on the 45 base: it
builds **musl** from source (clang `-g -O0`), bakes the out-of-source meson build
of the apue.3e examples, and runs `meson test` as a build gate — all passed.
Image `localhost/apue:latest`.

The C23 examples (modernized tree) and musl compiled clean under GCC 16 / clang
LLVM 23 at `warning_level=2` + `werror=true` — no new `-Werror` breakage. (The
`Failed to connect to audit log` lines in the test output are benign runtime
noise from the example programs, not test failures.)
