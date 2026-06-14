# Plan 2 — Replace recursive Make with Meson

## Goal

Replace `apue.3e/Makefile`, `apue.3e/Make.defines.*`,
`apue.3e/Make.libapue.inc`, and the per-directory `Makefile`s with a
Meson + Ninja build that:

- Builds `libapue.a` once at the top level.
- Builds every example program as its own executable, linked against
  `libapue.a`.
- Targets only **Linux (musl-clang, as in the Dockerfile)** and
  **macOS (system clang)**. FreeBSD and Solaris paths get deleted, not
  ported.
- Produces the same set of binaries the recursive Make build produces
  today, in `build/<dir>/<prog>` instead of in-tree.
- Is invoked from the existing podman Dockerfile / `Makefile` shell
  with one configure step and one compile step.

## Why Meson

- Real dependency graph; `ninja` parallelizes correctly across
  directories without the `for i in $DIRS; do (cd $$i && make); done`
  serialization.
- One build dir, easy to wipe — no `*.o` / executable scatter through
  the source tree.
- Native cross-platform support for Linux and macOS without the
  `Make.defines.<platform>` swap.
- Native files cleanly express "use musl-clang" without polluting
  source.
- Generated-source patterns (`rot13c2.c.in` → `rot13c2.c`) and
  multi-source executables (`pty/`, `printer/`) are first-class.

## Scope cuts

- **Drop** `Make.defines.freebsd`, `Make.defines.solaris`, `systype.sh`,
  the `solaris` and `freebsd` branches in every per-directory Makefile.
- **Drop** the `_XOPEN_SOURCE 600` Solaris branch in `apue.h`.
- **Keep** the `MACOS` / `LINUX` define semantics so source-level
  `#ifdef`s still work.

## Top-level layout

```
apue.3e/
  meson.build                       # project(), compiler args, libapue dep
  meson.options                     # toggles (e.g. enable_printer)
  native-linux.ini                  # CC=musl-clang
  native-macos.ini                  # CC=clang (default)
  include/
    meson.build                     # exports include dir as a dep
  lib/
    meson.build                     # static_library('apue', ...)
  intro/
    meson.build                     # foreach prog in [...]: executable(...)
  ... (one meson.build per program directory)
```

## Top-level `meson.build` sketch

```meson
project('apue', 'c',
  version : '3e',
  default_options : [
    'c_std=c99',          # -ansi was c89; bump to c99 for // and decls-mid-block
    'warning_level=2',    # ~ -Wall -Wextra
    'buildtype=debug',    # -g -O0
    'b_ndebug=false',
  ],
)

cc = meson.get_compiler('c')
host_os = host_machine.system()

common_args = []
common_link_args = []

if host_os == 'linux'
  common_args += ['-DLINUX', '-D_GNU_SOURCE']
elif host_os == 'darwin'
  common_args += ['-DMACOS', '-D_DARWIN_C_SOURCE']
else
  error('unsupported platform: ' + host_os
      + ' — this build supports linux and macos only')
endif

add_project_arguments(common_args, language : 'c')

apue_inc = include_directories('include')

subdir('include')
subdir('lib')      # produces libapue_dep

# Example program directories
example_dirs = [
  'intro', 'stdio', 'fileio', 'filedir', 'environ', 'proc',
  'signals', 'relation', 'daemons', 'threads', 'threadctl',
  'ipc1', 'ipc2', 'sockets', 'pty', 'termios', 'advio',
  'db', 'datafiles', 'exercises', 'figlinks', 'printer',
  'standards',
]
foreach d : example_dirs
  subdir(d)
endforeach
```

## `lib/meson.build` sketch

```meson
libapue_sources = files(
  'bufargs.c', 'cliconn.c', 'clrfl.c',
  'daemonize.c', 'error.c', 'errorlog.c',
  'lockreg.c', 'locktest.c',
  'openmax.c', 'pathalloc.c', 'popen.c',
  'prexit.c', 'prmask.c',
  'ptyfork.c', 'ptyopen.c',
  'readn.c', 'recvfd.c',
  'senderr.c', 'sendfd.c',
  'servaccept.c', 'servlisten.c',
  'setfd.c', 'setfl.c',
  'signal.c', 'signalintr.c',
  'sleepus.c', 'spipe.c', 'tellwait.c',
  'ttymodes.c', 'writen.c',
)
# Note: strerror.c is intentionally NOT in this list — matches the
# current Makefile. sleep.c is a separately-built .o today; if anyone
# ever links it, expose it as a separate static_library.

libapue = static_library('apue', libapue_sources,
  include_directories : apue_inc,
)

libapue_dep = declare_dependency(
  link_with : libapue,
  include_directories : apue_inc,
)
```

## Per-directory pattern

For a typical "each .c is its own program" directory like `intro/`:

```meson
progs = ['getcputc', 'hello', 'ls1', 'mycat', 'shell1', 'shell2',
         'testerror', 'uidgid']
foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach
```

For a directory that wants `-pthread`, declare a local dep:

```meson
threads = dependency('threads')   # picks -pthread
rt = host_os == 'linux' ? cc.find_library('rt', required : true) : []

extra = [threads]
if host_os == 'linux'
  extra += [rt]
endif

base_progs = ['badexit2', 'cleanup', 'exitstatus', 'threadid']
foreach p : base_progs
  executable(p, p + '.c', dependencies : [libapue_dep] + extra)
endforeach

# Linux-only barrier example
if host_os == 'linux'
  executable('barrier', 'barrier.c',
    dependencies : [libapue_dep] + extra)
endif

# Non-macOS: pthread_mutex_timedlock
if host_os != 'darwin'
  executable('timedlock', 'timedlock.c',
    dependencies : [libapue_dep] + extra)
endif
```

`condvar.c`, `maketimeout.c`, `mutex1.c`, `mutex2.c`, `mutex3.c`,
`rwlock.c` are compile-only in the current Makefile (no executable).
Express them as `static_library('threads_examples', [...])` with
`build_by_default: true` so the same compile coverage is preserved,
or as a `compile-only` custom target. **Decide which during phase 1**
— probably the static-library form is simplest.

## Special cases

### `advio/rot13c2.c.in` → `rot13c2.c`

Current rule:
```
rot13c2.c: rot13c2.c.in $(LIBAPUE)
	./fixup.awk rot13a.c >xlate
	sed '/same/q' rot13c2.c.in >rot13c2.c
	cat xlate >>rot13c2.c
	sed '1,/same/d' rot13c2.c.in >>rot13c2.c
```

Meson translation: a `custom_target('rot13c2_c', ...)` that depends on
`rot13c2.c.in`, `rot13a.c`, and `fixup.awk`, runs a small shell wrapper
script (`scripts/gen_rot13c2.sh`) that performs the awk + sed pipeline,
and emits `rot13c2.c` into the build dir. Then a normal
`executable('rot13c2', custom_target_output)`.

Wrap the pipeline in a script rather than inline — Meson `custom_target`
commands run a single argv, not a shell snippet.

### `pty/` — multi-source executable

`pty/Makefile` links `driver.c`, `loop.c`, `main.c` (and presumably
`ptybuf.c` / similar) into one binary. Express as
`executable('pty', files('main.c', 'loop.c', 'driver.c'),
dependencies: libapue_dep)`.

### `printer/` — cross-directory `.o` linking

The current Makefile reaches across directories:
```
print:  print.o util.o $(ROOT)/sockets/clconn2.o $(LIBAPUE)
printd: printd.o util.o $(ROOT)/sockets/clconn2.o $(ROOT)/sockets/initsrv2.o ...
```

Two ways to handle this in Meson:
1. **Refactor**: move `clconn2.c` and `initsrv2.c` into a small static
   library (`socket_helpers`) declared in `sockets/meson.build`,
   consume it as a `dependency()` in `printer/meson.build`. Cleaner.
2. **Direct**: pass the source files into `printer`'s
   `executable()` directly, e.g.
   `executable('print', files('print.c', 'util.c',
   '../sockets/clconn2.c'), dependencies: libapue_dep)`. Quicker, but
   recompiles those translation units twice.

Pick option 1 — it's the right shape and avoids surprising rebuilds.

### `ipc2/` nested subdirectories

`ipc2/open`, `ipc2/opend`, `ipc2/open.fe`, `ipc2/opend.fe` each have
their own Makefile. Express each as a `subdir()` from
`ipc2/meson.build`. Each emits its own executable.

The `.fe` directories contain the front-end variants (open via
fattach/door on Solaris originally) — confirm they actually build on
Linux + macOS before including; if not, fence them with
`if host_os == 'linux'` or drop entirely.

### `ipc2` / `sockets` Linux-only fd-passing

`recvfd2`, `sendfd2` are gated on Linux/FreeBSD in the Makefile. Keep
the Linux gate; drop the FreeBSD branch.

### `threads/` macOS exclusions

`timedlock` is excluded on macOS (no `pthread_mutex_timedlock`).
`barrier` is Linux-only. Express via `if host_os == ...` as shown above.

### Header-only directories / non-buildable directories

- `figlinks/` — likely just symlinks to other example sources for
  the book's figure numbering. If so, no `meson.build` needed and the
  directory drops out of `example_dirs`.
- `standards/` — `conf.c.modified` is the buildable file; rename it
  to `conf.c` (or generate `conf.c` from `.modified` via
  `configure_file()`) so meson can compile it. The two `*.awk`
  files are not part of the build.

## Compiler flags translation

| Make flag                     | Meson equivalent                                  |
|-------------------------------|---------------------------------------------------|
| `-ansi`                       | `c_std=c99` (or `c89` to preserve original)       |
| `-Wall`                       | `warning_level=2`                                 |
| `-g -O0`                      | `buildtype=debug`                                 |
| `-DLINUX` / `-DMACOS`         | conditional `add_project_arguments`               |
| `-D_GNU_SOURCE`               | linux branch of conditional above                 |
| `-D_DARWIN_C_SOURCE`          | macos branch of conditional above                 |
| `-I$(ROOT)/include`           | `apue_inc = include_directories('include')`       |
| `-L$(ROOT)/lib -lapue`        | `libapue_dep`                                     |
| `EXTRALIBS=-lrt`              | `cc.find_library('rt')`                           |
| `EXTRALIBS=-pthread`          | `dependency('threads')`                           |

The `-ansi` → `c89` choice is worth a deliberate decision. The book's
code is C89 with some POSIX extensions; bumping to `c99` or `c11`
makes warnings cleaner but slightly diverges from the book's text.
**Preserve `c89` initially** to minimize behavior change; revisit
once the build is green.

## Native files

`native-linux.ini`:

```ini
[binaries]
c = '/apue/musl/bldInstall/bin/musl-clang'
ar = 'ar'
strip = 'strip'

[built-in options]
c_args = []
c_link_args = []
```

`native-macos.ini`: omit or use the system defaults; macOS picks up
clang from `xcrun`.

Configure step in the container:
```sh
meson setup build --native-file native-linux.ini
meson compile -C build
```

On macOS:
```sh
meson setup build
meson compile -C build
```

## Container & Makefile changes

### `Dockerfile`

Add to the `dnf install` block:
```
meson \
ninja-build \
```

(Both are packaged in Fedora 44.)

### Top-level `/apue/Makefile`

Replace the `format` and add a `build` target that runs meson inside
the container:

```make
.PHONY: build
build: image  ## Configure + compile via meson
	$(CONTAINER_CMD) run -it --rm \
		--entrypoint /bin/bash \
		$(FILES_TO_MOUNT) \
		$(CONTAINER_NAME) \
		-c 'cd /apue/apue.3e && \
		    meson setup --reconfigure build \
		         --native-file native-linux.ini && \
		    meson compile -C build'
```

Leave the `shell` target alone for interactive use; the `build` target
is for CI / one-shot builds.

### `entrypoint/shell.sh`

Optionally `meson setup build --native-file native-linux.ini` on first
run if `build/` doesn't exist, so `meson compile -C build` "just works"
inside the interactive shell.

## Migration strategy (phased — keeps trunk shippable)

**Phase 0 — branch, baseline.** Capture a known-good `make all` log
on `master` for both Linux (in container) and macOS. This is the
target output to match.

**Phase 1 — meson alongside make.** Add `apue.3e/meson.build` and
all per-directory `meson.build` files. Do *not* delete the Makefiles
yet. Verify `meson compile -C build` produces an executable for every
program in the existing baseline and that they all run their
"hello-world" smoke test (e.g. `intro/hello`, `intro/ls1 .`).

**Phase 2 — diff binary lists.** Script comparing the set of
executables produced under the make build vs. the meson build.
Reconcile any missing or extra targets.

**Phase 3 — wire CI / Dockerfile.** Switch the Dockerfile to install
meson + ninja and switch `make build` to call meson. Keep the make
files in-tree as a fallback for one release.

**Phase 4 — delete the old build.** Remove `Makefile`,
`Make.defines.*`, `Make.libapue.inc`, `systype.sh`, every
per-directory `Makefile`, `lib/Makefile`. Update `TODO.org`.

## Verification

- `meson compile -C build` succeeds with no warnings beyond the
  baseline captured in phase 0.
- Spot-run smoke programs:
  ```
  build/intro/hello
  build/intro/ls1 /tmp
  build/fileio/seek
  build/threads/threadid
  ```
- For each directory listed in the original `Makefile`'s `DIRS`, the
  executables produced by meson exactly match the set the make build
  produced.
- `meson test -C build` — register the smoke programs as `test()`
  targets so future regressions get caught.

## Risks / open questions

- **`-ansi` vs. `c_std`**: meson's `c89` doesn't pass `-ansi`
  literally on every compiler; close but not identical. If a book
  example relies on `__STRICT_ANSI__` behavior we may need to add
  `-ansi` via `add_project_arguments`. Unlikely.
- **musl-clang in meson**: the wrapper script is just a clang
  invocation with different sysroot flags. Meson should treat it as
  any other C compiler; native file should suffice. If not,
  fall back to setting `CC=musl-clang` in the environment before
  `meson setup`.
- **`fixup.awk` portability**: relies on the old `awk`. If meson runs
  in an environment where `awk` is `mawk` or `gawk` with different
  defaults, the generated `rot13c2.c` may differ. Lock to `gawk` if
  needed.
- **`printer/`'s IPP code** may pull in dependencies not present on
  macOS. If so, gate the whole `printer/` directory on Linux only —
  the book itself notes printer chapter is Linux-leaning.
- **Directory `figlinks/`** — confirm it's just symlinks before
  excluding from the build.
- **`db/`** has both example programs and a small DB library — check
  whether `db.c` should be a static_library that `t4.c` links against,
  or two independent compilations.

## Out of scope

- Switching to CMake or autotools (explicitly rejected — meson chosen).
- Cross-compilation, sanitizer presets, install rules, packaging.
- Replacing the Dockerfile / podman setup.
- Touching the source code itself (that's plan 1).
