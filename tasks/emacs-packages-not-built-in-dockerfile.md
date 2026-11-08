# Emacs packages are not installed or compiled in the Dockerfile

**Status:** in progress
**Priority:** 3
**Difficulty:** 2
Created 2026-11-08 (William Emerison Six <billsix@gmail.com> asked for it).
**Part of:** github.com/billsix/runClaudeInContainer `tasks/emacs-contract-conformance.md` (item 1, the
build-time MELPA install, and item 5, the run-time compile).

## BLUF

Under `USE_EMACS=1` the `Dockerfile` runs `emacs --batch --load install-melpa-packages.el`, downloading
and byte-compiling the whole MELPA set at image build, and `make shell` then mounts the vendored
`entrypoint/dotfiles/.emacs.d/` over `/root/.emacs.d`, so the result is never used. Done = the image
installs only the Emacs binary, under `USE_EMACS`; `init.el` byte-compiles the vendored packages on
Emacs's first start; `make image` builds with no MELPA fetch; Emacs starts cleanly against the mount.

## Context

- The Emacs contract: the `emacs-container-contract.md` reference doc in github.com/billsix/runClaudeInContainer
  `tasks/reference/`. Worked C example: github.com/billsix/Craft
  `tasks/archive/2026/11/08/emacs-packages-not-built-in-dockerfile.md`.
- Today: `emacs` is in the `Dockerfile`'s inline `dnf install` list (so `USE_EMACS=0` still installs it);
  a later `ARG USE_EMACS` + `COPY install-melpa-packages.el` + `RUN emacs --batch …` sits between the musl
  build and the `apue.3e` `COPY`; the Makefile mounts the whole `.emacs.d/`; `init.el` has no run-time
  compile; `.gitignore` already ignores `*.elc`/`*.eln`.

## Decisions

- **Install inline, not via a new `0N-install-emacs.sh`.** apue has no `0N-install-*.sh` group scripts:
  its packages are installed inline in the `Dockerfile`. Extracting them is a separate contract item, so
  this task replaces the MELPA `RUN` with a `USE_EMACS`-gated `dnf install -y emacs` in the same spot
  (after the musl build, before the `apue.3e` `COPY`) and drops `emacs` from the base list.
- **Keep the whole-`.emacs.d` mount.** The image bakes no Emacs config, so mounting only `elpa/` would
  need a new dotfiles `COPY`; the existing mount keeps config edits live without `make image`.

## Plan

1. `Dockerfile`: remove `emacs` from the base list; replace the MELPA `COPY` + `RUN` with a
   `USE_EMACS`-gated `dnf install -y emacs` (dnf cache mounts kept).
2. `init.el`: `(byte-recompile-directory package-user-dir 0)` after `(package-initialize)`.
3. Makefile comments, `CLAUDE.md`, `README.md`: describe the new behavior.

## Verification

- `make image` (default flags): no MELPA fetch; `/usr/bin/emacs` present; no `/root/.emacs.d/elpa`.
- `make shell-exec CMD='emacs --batch -l /root/.emacs.d/init.el …'`: exits 0, compiles `.elc` into the
  host `elpa/`, `git status` clean.
- `make image USE_EMACS=0`: no `emacs` on `PATH`.
- `tools/check_helm_file_at_point.sh` (runClaudeInContainer): 7/7.

## Open questions

None.
