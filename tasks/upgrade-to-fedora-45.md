# Upgrade the container base from Fedora 44 to Fedora 45

**Status:** in progress
**Priority:** 3
**Difficulty:** 4

## BLUF

Bump `Dockerfile` from `registry.fedoraproject.org/fedora:44` to `fedora:45` and
get `make image` green nested. The image build is a gate: it builds **musl** from
source, then `meson setup/compile/test` of the apue.3e examples. "Done" = the
image builds and `meson test` passes on the 45 base.

## Context (read first)

Part of a fleet-wide Fedora 44 → 45 bump (William Emerison Six
<billsix@gmail.com>, 2026-11-05). Fedora 45 ships **GCC 16.2 / clang from LLVM
23, glibc 2.44**. Risks here: (1) building musl-from-source under GCC 16;
(2) the apue.3e example sources (older Unix C) under the newer compiler — though
the meson build's warning posture, not `-Werror` by default, may absorb new
warnings. The inline `dnf install` already uses `--skip-unavailable`, so a
renamed/dropped package degrades gracefully rather than failing the build.

- Build gate: `make image` nested (`PODMAN_RUN_FLAGS` auto-adds
  `--cgroups=disabled`).

## Plan

1. `Dockerfile`: `fedora:44` → `fedora:45`.
2. `make image` nested; read `--print-errorlogs` on failure.
3. Fix any new toolchain breakage minimally (discretion; log decisions here).
4. Re-run until green; stage by path; archive.

## Notes / findings

(appended as the build runs)
