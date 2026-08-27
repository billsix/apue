# (Question) Should the example binaries be installed onto PATH?

**Status:** blocked
**Priority:** 8
**Difficulty:** 2
**Started:** 2026-08-27 (William Emerison Six <billsix@gmail.com>)
**Blocked on:** maintainer answers the single Open question below. **This task may be DROPPED** — the
literal question the bullet asked is already answered "no problem"; it only survives as a task if the
maintainer actually wants the *opposite* (examples on PATH) as a feature.
**Recheck:** the Open question below is answered (maintainer-gated; `/recheck-blocked` surfaces it).

## Goal

Maintainer's idea, verbatim: *"Check dockerfile, Is it trying to install the examples into the main
executable path?"*

## Investigation result (2026-08-27) — the literal question is answered: NO

The Dockerfile does **not** install the example binaries into any system executable path
(`/usr/local/bin`, `/usr/bin`, …). Evidence:

- `Dockerfile:33-37` — the only `make install` installs **musl libc** to the private prefix
  `/apue/musl/bldInstall` (`Dockerfile:36`), not the examples.
- `Dockerfile:44` — `COPY apue.3e /apue/apue.3e/` copies **source only**; the Dockerfile never compiles
  or installs the examples at image-build time.
- Examples are compiled **out-of-tree** into `apue.3e/build/` at runtime via `make build`
  (`Makefile:63-76`); there is **no** `install:` in any `apue.3e/**/meson.build`, and no
  `meson install`/`ninja install` anywhere. Example binaries never land on `PATH`.

**So: nothing to fix.** If the maintainer was just checking it isn't polluting `/usr/local/bin`, the
answer is "it isn't" and this task should be **dropped** (`/stack-drop`-style — record why).

## Open question

1. Did you want the **opposite** — the example binaries installed / on `PATH` for convenience (making
   this a NEW feature request), or were you just confirming the Dockerfile isn't polluting the system
   executable path (in which case: no action, drop this task)? *(Recommend: no action / drop, unless you
   want the convenience feature.)*
