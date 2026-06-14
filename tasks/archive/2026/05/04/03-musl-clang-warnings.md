# Plan 3 — Silence musl-clang "argument unused during compilation" noise

## Problem

Every `.c` compilation through `musl-clang` emits 5 warning lines:

```
clang: warning: argument unused during compilation: '-fuse-ld=musl-clang' [-Wunused-command-line-argument]
clang: warning: argument unused during compilation: '-static-libgcc' [-Wunused-command-line-argument]
clang: warning: argument unused during compilation: '-L-user-start' [-Wunused-command-line-argument]
clang: warning: argument unused during compilation: '-L/apue/musl/bldInstall/lib' [-Wunused-command-line-argument]
clang: warning: argument unused during compilation: '-L-user-end' [-Wunused-command-line-argument]
```

## Cause

`/apue/musl/tools/musl-clang.in` (lines 23-35) — the wrapper template that
`configure` substitutes into the installed `musl-clang` script — passes
linker flags on every invocation:

```sh
exec $cc \
    -B"$thisdir" \
    -fuse-ld=musl-clang \
    -static-libgcc \
    -nostdinc \
    --sysroot "$libc" \
    -isystem "$libc_inc" \
    -L-user-start \
    $sflags \
    "$@" \
    $eflags \
    -L"$libc_lib" \
    -L-user-end
```

The wrapper has conditional logic to wrap user `-l<lib>` flags between
`-l-user-start` / `-l-user-end` sentinels (lines 9-21), but the `-L`
flags, `-fuse-ld`, and `-static-libgcc` are unconditional. clang sees
them on `-c`-only invocations and correctly notes they have no effect.

## Does it need fixing?

The warnings are *accurate*. The flags really are unused. Compilation
output is correct. So nothing is broken.

However:
- They drown out real warnings (the real `_POSIX_C_SOURCE` /
  `_XOPEN_SOURCE` redefinitions from `apue.h`, plus any new warnings
  from this project's code).
- Each warning is 5 lines × every `.c` file compiled, so a full
  `make` is ~1000+ lines of noise for ~200 source files.
- They make the build look broken to a casual reader, especially a
  beginner — which clashes with this project's pedagogical bent.

Verdict: **fix it — it costs nothing.**

## Options considered

### A. Pass `-Wno-unused-command-line-argument` in `CFLAGS` (recommended)

One line in `apue.3e/Make.defines.linux`:

```make
CFLAGS=-ansi -I$(ROOT)/include -Wall -DLINUX -D_GNU_SOURCE \
       -Wno-unused-command-line-argument $(EXTRA) -g -O0
```

Pros:
- Trivial, reversible, isolated.
- Doesn't touch musl source or the Dockerfile.
- Doesn't suppress the warning class for the *user's* code — only for
  flags that musl-clang itself injects. (Caveat: it also suppresses the
  warning if the user passes a stray flag, but in this build harness
  that's extremely unlikely and not worth designing around.)

Cons:
- Treats the symptom, not the cause.
- Doesn't help anyone outside this project who uses musl-clang.

### B. Patch `musl-clang.in` to detect compile-only mode

Add logic that scans `"$@"` for `-c`/`-S`/`-E` (compile-only modes)
and omits linker flags when present:

```sh
linking=1
for x ; do
    case "$x" in
        -c|-S|-E|-M|-MM) linking= ;;
    esac
done

if test "$linking" ; then
    exec $cc -B"$thisdir" -fuse-ld=musl-clang -static-libgcc \
             -nostdinc --sysroot "$libc" -isystem "$libc_inc" \
             -L-user-start $sflags "$@" $eflags \
             -L"$libc_lib" -L-user-end
else
    exec $cc -B"$thisdir" -nostdinc --sysroot "$libc" \
             -isystem "$libc_inc" "$@"
fi
```

Pros:
- Fixes the root cause. musl-clang stops emitting nonsense flags.
- Future-proof against other compile-only invocations.

Cons:
- Patches musl source (`/apue/musl/tools/musl-clang.in`). The musl
  directory is under git; the patch would carry forward but diverges
  from upstream musl.
- Has to be applied before `configure` runs in the Dockerfile (it
  already is — the `COPY musl /apue/musl/musl` step copies in source,
  then runs `configure`).
- Slightly more risk: misclassifying a link invocation breaks linking.
  The flag list `-c -S -E -M -MM` covers the standard compile-only
  flags but is not exhaustive (e.g. `--analyze` etc.). Low risk in
  practice for this project.

### C. Bypass musl-clang for compilation, use it only for linking

Set `CC=clang` (system clang) for compilation, set a separate `LINK.c`
that uses `musl-clang`. Requires reworking `Make.defines.linux` and
losing musl's `-isystem` for headers during compile. Bad fit — drop.

## Recommendation

Take option A as a one-line change now. The musl-clang wrapper is
upstream code; carrying a local patch (option B) is a maintenance cost
for a cosmetic problem. If the warning noise turns out to also affect
the planned Meson build (plan 2), revisit at that point — the warnings
will appear there too, and the same `-Wno-unused-command-line-argument`
in the project args block will silence them.

## Steps for option A

1. Edit `/apue/apue.3e/Make.defines.linux`: append
   `-Wno-unused-command-line-argument` to the `CFLAGS` line.
2. Rebuild and confirm warnings are gone.
3. Leave `Make.defines.macos` alone — system clang on macOS is invoked
   directly, no wrapper, no spurious flags.

## Out of scope

- Fixing musl upstream.
- Anything that requires touching `/apue/musl/`.
