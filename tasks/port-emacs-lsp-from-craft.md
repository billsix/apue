# Port the in-container Emacs + LSP (clangd) developer setup from Craft

**Status:** in progress — infrastructure implemented & staged 2026-10-20; remaining steps are the maintainer's
**Priority:** 5
**Difficulty:** 4 (mostly mechanical copy + path edits; the one real unknown is clangd vs. the musl-clang wrapper — see step 4)
**Created:** 2026-10-20 (William Emerison Six <billsix@gmail.com>: "make a task to port emacs lsp
into apue, like it's done in Craft, with LSP, makefile target for 'make update-emacs-packages' etc")
**Open questions resolved** 2026-10-20 (William Emerison Six <billsix@gmail.com>): (1) lsp-mode +
clangd, as Craft; (2) full MELPA set, copied verbatim. Scope clarified: copy the *infrastructure*
(Makefile/Dockerfile/mount plumbing) + the hand-written `.el` config, **not** Craft's vendored
`elpa/` lisp — the maintainer runs `make update-emacs-packages` to fetch those.

## Progress (2026-10-20)

**Done and staged** (steps 1–7 below): the four hand-written config files
(`entrypoint/dotfiles/.emacs.d/{init.el,preferences.el,helm.el,install-melpa-packages.el}`,
`init.el` adapted to apue's `/apue/apue.3e/` root + `--compile-commands-dir=/apue/apue.3e/build`
+ the musl-clang `--query-driver=/apue/musl/bldInstall/bin/*,/usr/bin/clang*`), the `Makefile`
`USE_EMACS ?= 1`/`ELPA_MOUNT` plumbing + `$(ELPA_MOUNT)` in `SHELL_RUN_FLAGS` + `--build-arg
USE_EMACS` on `image` + the `update-emacs-packages` target, the `Dockerfile` `USE_EMACS` bake block
(placed after the musl build, before the source COPY, for layer caching), and the `.gitignore`
`*.elc`/`*.eln`/`eln-cache/` entries. `make help` parses; the Dockerfile compiles under `make image
USE_EMACS=0`.

> **Behavior change to know (required reading before you next `make image`):** `USE_EMACS` now
> defaults to `1`, so a plain `make image`/`make build` will try to install the MELPA set at
> image-build time — **that needs network**. Pass `USE_EMACS=0` to skip it (the C build doesn't need
> Emacs). This matches Craft's default.

**Remaining — the maintainer's (needs network and/or a live Emacs):**
- Run `make update-emacs-packages` once to generate apue's own committed `entrypoint/dotfiles/.emacs.d/elpa/` pin.
- Verify (BLUF acceptance): `make build` once (writes `compile_commands.json`), then `make shell`,
  open a `.c` under `/apue/apue.3e/`, confirm clangd attaches AND resolves a **musl libc header**
  with no spurious "file not found" (that's the `--query-driver` check — the one genuinely new piece).
- If clangd still can't find musl headers, see Open question 1 below (may need an extra `--isystem`).
- Add the short Emacs-workflow note to `CLAUDE.md`/`README` (step 9) once it's confirmed working.

## BLUF

Craft ships an in-container Emacs that attaches `lsp-mode` → `clangd` to C buffers, with a
committed `elpa/` package pin refreshed by a `make update-emacs-packages` target. apue has none of
this (its `entrypoint/dotfiles/` holds only `.extrabashrc` and `.lldbinit`). **"Done" = from a
fresh `make shell` in apue you can open a `.c` under `/apue/apue.3e/`, `lsp-mode` auto-attaches
clangd, and completion / diagnostics / `M-.` jump work against the real Meson build; plus a
`make update-emacs-packages` target that rebuilds the image, reinstalls the MELPA set, strips
`*.elc`/`*.eln`, and *stages* the refreshed `elpa/` (stage-then-stop, never auto-commit).** apue's
image already has `emacs`, `clang-tools-extra` (clangd), `meson`, and `ninja-build`, so this is
additive wiring, not a toolchain change.

## Context (read first)

The reference implementation is **Craft** (canonical remote
`github.com/billsix/Craft`; shipped 2026-10-17 in its
`tasks/archive/2026/10/17/add-container-template.md`). Craft mirrors the `billsEmacsConfigs/c` and
`gltron` templates. The pieces, all under Craft's `entrypoint/dotfiles/.emacs.d/`:

- **`init.el`** — `lsp-mode` (NOT eglot) for C/C++, `clangd` on PATH, `lsp-ui`, `company`,
  `clang-format` on `C-c f`. It points clangd at the build **two ways**: a clangd arg
  `--compile-commands-dir=/craft/builddir`, and an `lsp--calculate-root` override returning the
  repo root. (eglot is used *only* for emacs-lisp, in `preferences.el`, so two clients never both
  attach to a C buffer.)
- **`preferences.el`** — editor prefs (`delete-trailing-whitespace` on save, line numbers,
  `eglot-ensure` for elisp only).
- **`helm.el`** — helm completion remaps.
- **`install-melpa-packages.el`** — the manifest: plain `package.el` + MELPA, a
  `package-selected-packages` list, install-if-missing. This is the source of truth for *which*
  packages; the committed `elpa/` tree is the *version pin*.
- **`elpa/`** — ~45 package source dirs committed to git (the lockfile; `*.elc`/`*.eln` are
  gitignored and stripped). Do NOT copy Craft's `elpa/` into apue — regenerate it with the update
  target so the pin reflects apue's own install.

apue's relevant current state (verified 2026-10-20):
- `Makefile`: `CONTAINER_NAME = apue`; `FILES_TO_MOUNT` mounts `./apue.3e` → `/apue/apue.3e:Z` and
  uses a `TMUX_MOUNT := $(shell if [ -f … ]; then echo "-v …:/root/.tmux.conf:Z"; fi)` pattern
  (the model to copy for an optional `.emacs.d` mount); `SHELL_RUN_FLAGS` includes `FILES_TO_MOUNT`;
  `make build` runs `cd /apue/apue.3e && meson setup build --native-file native-linux.ini && meson
  compile -C build && meson test -C build`, so **`compile_commands.json` is written to
  `/apue/apue.3e/build/` automatically** (Meson always emits it).
- `Dockerfile`: already `dnf`-installs `clang-tools-extra`, `emacs`, `meson`, `ninja-build`; it
  `COPY`s `entrypoint/dotfiles/.lldbinit` to `/root/.lldbinit`. HOME is `/root`.

## Plan

1. **Copy the four hand-written config files** from `github.com/billsix/Craft`
   `entrypoint/dotfiles/.emacs.d/{init.el,preferences.el,helm.el,install-melpa-packages.el}` into
   apue at the same relative path. Trim any Craft/game-specific packages from
   `install-melpa-packages.el` if they're clearly GL/game-only (optional; harmless to keep).
2. **Edit `init.el` for apue's paths** (the two Craft hardcodes):
   - `--compile-commands-dir=/craft/builddir` → `--compile-commands-dir=/apue/apue.3e/build`.
   - the `lsp--calculate-root` override → return `"/apue/apue.3e/"`.
3. **Add the `.emacs.d` bind-mount** to the Makefile, following the existing `TMUX_MOUNT` pattern:
   an `ELPA_MOUNT = -v $(CURDIR)/entrypoint/dotfiles/.emacs.d/:/root/.emacs.d/:U,z` gated on a
   `USE_EMACS ?= 1`, added into `SHELL_RUN_FLAGS` so `make shell` / `shell-exec` pick it up.
4. **Point clangd at the musl-clang wrapper — the one genuinely new piece (do this or diagnostics
   lie).** apue compiles through a two-stage musl-clang wrapper selected by
   `apue.3e/native-linux.ini` (musl installed in the image at `/apue/musl/bldInstall`), so
   `compile_commands.json` records that wrapper as the compiler. **clangd refuses to run an unknown
   driver to extract its built-in include paths unless allowlisted**, so without this it shows
   spurious "file not found" on musl headers. Add a `--query-driver=` clangd arg whose glob matches
   the wrapper path as it appears in `compile_commands.json` (inspect that file first; likely
   `--query-driver=/apue/musl/bldInstall/bin/*,/usr/bin/clang*`). Craft doesn't need this (plain
   system clang), so it's not in the copied `init.el` — add it. **Verify by opening a musl-header
   include in Emacs and confirming no false "not found,"** not just that Emacs launches.
5. **Add the Dockerfile `USE_EMACS` provisioning** (mirror Craft): `ARG USE_EMACS=0`, `COPY` the
   `.emacs.d` (at least `install-melpa-packages.el`) to `/root/.emacs.d/`, and a guarded
   `RUN … if [ "$USE_EMACS" = "1" ]; then emacs --batch --load …/install-melpa-packages.el; fi`.
   apue already installs `emacs`/`clang-tools-extra`, so only the package-install step is new. The
   runtime bind-mount (step 3) wins over the baked copy.
6. **Add the `make update-emacs-packages` target** (adapt Craft's verbatim recipe — see below) to
   apue's container name and the `/root/.emacs.d/` mount: `make image USE_EMACS=1`, then a
   throwaway `podman run` that wipes only `elpa/`, reinstalls from MELPA via `emacs --batch`, then
   on the host strips `*.elc`/`*.eln` and `git add -A -f` the `elpa/` tree. Needs network. **Stages,
   does not commit** (matches the repo convention).
7. **Gitignore** `*.elc`, `*.eln`, `eln-cache/` under `entrypoint/dotfiles/.emacs.d/`.
8. **Run `make update-emacs-packages` once** to generate apue's own committed `elpa/` pin, then the
   acceptance check in BLUF (open a `.c`, confirm clangd attaches and resolves musl headers).
9. **Doc**: a short `README`/`CLAUDE.md` note on the Emacs workflow (`make build` once to generate
   `compile_commands.json`, then `make shell`, then `emacs -nw <file>`), and a one-line pointer in
   `CLAUDE.md`'s container-workflow section.

## Craft's `update-emacs-packages` recipe (verbatim — adapt CONTAINER_NAME/paths)

```makefile
update-emacs-packages:
	$(MAKE) image USE_EMACS=1
	$(CONTAINER_CMD) run --rm $(PODMAN_RUN_FLAGS) \
		-v $(CURDIR)/entrypoint/dotfiles/.emacs.d/:/root/.emacs.d/:U,z \
		--entrypoint /bin/bash \
		$(CONTAINER_NAME) \
		-c 'mkdir -p /root/.emacs.d/elpa; find /root/.emacs.d/elpa -mindepth 1 -delete; \
		    emacs --batch --load /root/.emacs.d/install-melpa-packages.el'
	mkdir -p $(CURDIR)/entrypoint/dotfiles/.emacs.d/elpa && cd $(CURDIR)/entrypoint/dotfiles/.emacs.d/elpa && \
		find . \( -iname '*.elc' -o -iname '*.eln' \) -delete && \
		git add -A -f .
	@echo "Done: reinstalled packages, stripped *.elc/*.eln, staged elpa -- review and commit."
```

## Open questions

1. **(Still open — resolves on the maintainer's live verification.)** clangd vs. the musl-clang
   wrapper (step 4): is `--query-driver` enough, or does the wrapper need an extra `--isystem`? Can't
   be settled without running it — the verification step (open a musl-header include in a live Emacs)
   is the gate. `--query-driver` is in `init.el`; if musl headers still 404, add the musl
   `-isystem /apue/musl/bldInstall/include` to clangd's args.
2. ~~Keep Craft's full MELPA set or trim?~~ **Resolved: full set, copied verbatim** (maintainer,
   2026-10-20).

## See also

- `github.com/billsix/Craft:entrypoint/dotfiles/.emacs.d/init.el` — the lsp-mode/clangd wiring to copy.
- `github.com/billsix/Craft:entrypoint/dotfiles/.emacs.d/install-melpa-packages.el` — the package manifest.
- `github.com/billsix/Craft:Makefile` — the `update-emacs-packages` target + `ELPA_MOUNT`/`USE_EMACS` plumbing.
- `github.com/billsix/Craft:Dockerfile` — the `USE_EMACS` provisioning block.
- `tasks/reference/build-toolchain-musl-clang.md` — why clangd needs the musl-clang driver allowlisted (the two-stage wrapper + native file).
- apue `Makefile` `TMUX_MOUNT` — the existing optional-bind-mount pattern to mirror for `.emacs.d`.
