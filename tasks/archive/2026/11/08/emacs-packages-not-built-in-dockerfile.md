# Emacs packages are not installed or compiled in the Dockerfile

Done 2026-11-08 (William Emerison Six <billsix@gmail.com> asked for it). Part of github.com/billsix/runClaudeInContainer
`tasks/emacs-contract-conformance.md` (the build-time MELPA install and the run-time compile items).

## What was wrong

Under `USE_EMACS=1` the `Dockerfile` ran `emacs --batch --load install-melpa-packages.el`: a network
download plus byte-compile of the whole MELPA set at image build. `make shell` then mounted the vendored
`entrypoint/dotfiles/.emacs.d/` over `/root/.emacs.d`, so the result was never used. `emacs` sat in the
inline base `dnf install`, so `USE_EMACS=0` still installed it. The README also told the reader to run
`make update-emacs-packages` before first use, although `elpa/` is vendored.

## What changed

- `Dockerfile`: `emacs` left the base list; the MELPA `COPY` + `RUN` became a `USE_EMACS`-gated
  `dnf install -y emacs` in the same place (after the musl build, before the `apue.3e` `COPY`), with the
  dnf cache mounts. apue installs packages inline (it has no `0N-install-*.sh` group scripts), so the
  binary install stays inline too; extracting the scripts is a separate contract item.
- `init.el` runs `(byte-recompile-directory package-user-dir 0)` after `(package-initialize)`; the first
  Emacs start in `make shell` compiles the vendored packages into the mounted host `elpa/` (gitignored).
- Kept the whole-`.emacs.d` mount: the image bakes no Emacs config, so config edits stay live.
- Makefile comments, `CLAUDE.md` and `README.md` describe this; the README's "first time, fetch the
  packages" step is gone (`update-emacs-packages` is now described as the refresh it is).

## Verification (2026-11-08, nested podman in the runClaudeInContainer sandbox)

- `make image` (default flags) succeeded with no MELPA fetch; the image-build `meson test` gate: 8 ok,
  0 fail. The image has `/usr/bin/emacs` and no `/root/.emacs.d`.
- Emacs in batch with the Makefile's `.emacs.d` mount, `--network=none`
  (runClaudeInContainer `tasks/adhoc/emacs-no-build-time-compile/emacs-start-check.sh`): exit 0, 317 `.elc`
  in the host `elpa/`; `git status` showed only this change (the `.elc` are ignored).
- `tools/check_helm_file_at_point.sh` (runClaudeInContainer): 7/7.
- Not run: `make image USE_EMACS=0`.
- Found, not fixed (contract-level): the run-time compile logs `Cannot open load file … compat-macs.el`
  for compat's `compat-26.el`…`compat-30.el` on every start; harmless on Emacs 31, recorded in the
  runClaudeInContainer umbrella.
