# Plan 1 — Inline real system headers, retire `apue2.h`

## Goal

Each `.c` file in `apue.3e/` should `#include` only the system headers it
actually uses, with a short trailing comment naming what is used. The
catch-all `apue2.h` then gets deleted.

This is a **teaching change**: a beginner reading `hello.c` should be able
to see *why* `<stdio.h>` is there. The book originally hid this behind a
single grab-bag header, which is convenient but pedagogically lossy.

## Current state (per `TODO.org` and commit `2cdfa09`)

- `apue.3e/include/apue.h` — stripped to APUE-only declarations / macros.
  Still references `off_t`, `pid_t`, `ssize_t`, `struct termios`,
  `struct winsize`, `S_I*` constants — so any TU including `apue.h`
  must first include `<sys/types.h>` (and `<sys/stat.h>` if it touches
  `FILE_MODE` / `DIR_MODE`, `<termios.h>` if it touches the pty
  prototypes, etc.).
- `apue.3e/include/apue2.h` — still exists, still pulled in by every
  unconverted `.c` file.
- Converted directories: `intro/`, `advio/`.
- Pending directories (24): `daemons`, `datafiles`, `db`, `environ`,
  `exercises`, `figlinks`, `filedir`, `fileio`, `ipc1`, `ipc2` (+ nested
  `open`, `opend`, `open.fe`, `opend.fe`), `lib`, `printer`, `proc`,
  `pty`, `relation`, `signals`, `sockets`, `standards`, `stdio`,
  `termios`, `threadctl`, `threads`.

## The pattern (already established in `intro/`, `advio/`)

```c
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for getpid */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"
```

Rules of thumb derived from the converted files:
- Each `#include` gets a trailing `/* for <symbols> */` comment.
- `<sys/types.h>` carries a `/* needed for apue.h */` comment when only
  pulled in to satisfy `apue.h`'s prototypes.
- A blank line separates system headers from project headers.

## Per-file procedure

For each `<file>.c`:

1. Note every symbol that comes from a system or library header
   (functions, types, macros, constants).
2. Map each symbol to its canonical POSIX/Linux header. Quick references:
   - `printf`, `fprintf`, `fputs`, `fflush`, `fopen`, `fgets`,
     `fread`, `fwrite`, `getc`, `putc`, `snprintf`, `vsnprintf`,
     `stdin`, `stdout`, `stderr`, `EOF`, `FILE` → `<stdio.h>`
   - `exit`, `atexit`, `malloc`, `free`, `getenv`, `setenv`, `system`,
     `abort` → `<stdlib.h>`
   - `strlen`, `strcat`, `strcpy`, `memset`, `memcpy`, `strerror` →
     `<string.h>`
   - `read`, `write`, `close`, `dup`, `dup2`, `pipe`, `fork`, `exec*`,
     `getpid`, `getppid`, `chdir`, `unlink`, `setsid`, `sleep`, `alarm`,
     `pause`, `getcwd`, `access`, `lseek`, `ftruncate`, `fsync` →
     `<unistd.h>`
   - `open`, `creat`, `O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_CREAT`,
     `O_TRUNC`, `O_APPEND`, `fcntl`, `F_*` → `<fcntl.h>`
   - `stat`, `fstat`, `lstat`, `umask`, `chmod`, `mkdir`, `S_I*`,
     `S_IS*` macros → `<sys/stat.h>`
   - `pid_t`, `uid_t`, `gid_t`, `off_t`, `size_t`, `ssize_t`, `mode_t`,
     `time_t`, `dev_t`, `ino_t` → `<sys/types.h>`
   - `wait`, `waitpid`, `WIFEXITED`, `WEXITSTATUS`, `WIFSIGNALED`,
     `WTERMSIG`, `WIFSTOPPED` → `<sys/wait.h>`
   - `signal`, `sigaction`, `sigemptyset`, `sigaddset`, `sigprocmask`,
     `kill`, `raise`, `SIG*` → `<signal.h>`
   - `time`, `localtime`, `gmtime`, `strftime`, `clock_gettime`,
     `nanosleep`, `struct tm`, `struct timespec`, `CLOCK_*` →
     `<time.h>`
   - `errno`, `EINTR`, `EAGAIN`, ... → `<errno.h>`
   - `va_list`, `va_start`, `va_end`, `va_arg` → `<stdarg.h>`
   - `setjmp`, `longjmp`, `sigsetjmp`, `siglongjmp` → `<setjmp.h>`
   - `pthread_*`, `PTHREAD_*` → `<pthread.h>`
   - `opendir`, `readdir`, `closedir`, `DIR`, `struct dirent` →
     `<dirent.h>`
   - `getrlimit`, `setrlimit`, `RLIMIT_*`, `struct rlimit` →
     `<sys/resource.h>`
   - `openlog`, `syslog`, `closelog`, `LOG_*` → `<syslog.h>`
   - `socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv`,
     `AF_*`, `SOCK_*` → `<sys/socket.h>`
   - `inet_pton`, `inet_ntop`, `htons`, `ntohs`, `htonl`, `ntohl` →
     `<arpa/inet.h>`
   - `struct sockaddr_in`, `struct in_addr`, `IPPROTO_*` →
     `<netinet/in.h>`
   - `getaddrinfo`, `freeaddrinfo`, `struct addrinfo` → `<netdb.h>`
   - `tcgetattr`, `tcsetattr`, `struct termios`, `cfmakeraw`, `B*`
     baud-rate macros → `<termios.h>`
   - `ioctl`, `TIOCGWINSZ`, `struct winsize` → `<sys/ioctl.h>`
     (already conditional in `apue2.h` — preserve that condition)
   - `isalpha`, `isdigit`, `tolower`, `toupper` → `<ctype.h>`
   - `mmap`, `munmap`, `PROT_*`, `MAP_*` → `<sys/mman.h>`
   - `semget`, `semop`, `shmget`, `shmat`, `msgget`, `msgsnd` →
     `<sys/sem.h>` / `<sys/shm.h>` / `<sys/msg.h>`
3. Replace `#include "apue2.h"` with the minimal set of system headers,
   each annotated.
4. Keep `#include "apue.h"` if and only if the file uses an APUE-defined
   symbol (`err_sys`, `err_quit`, `err_dump`, `err_ret`, `err_msg`,
   `err_cont`, `err_exit`, `MAXLINE`, `FILE_MODE`, `DIR_MODE`,
   `TELL_*`, `WAIT_*`, `read_lock`/`write_lock` macros, `signal_intr`,
   `pr_exit`, `pr_mask`, `daemonize`, `tty_*`, `pty*`, `cli_conn`,
   `serv_listen`, `serv_accept`, `send_fd`, `recv_fd`, `send_err`,
   `buf_args`, `path_alloc`, `open_max`, `set_cloexec`,
   `set_fl`/`clr_fl`, `readn`/`writen`, `sleep_us`, `fd_pipe`,
   `lock_reg`, `lock_test`, `log_*`).
5. If `apue.h` is included, ensure `<sys/types.h>` (and any other type
   header the relevant prototypes need) precedes it.
6. Check that the file builds: `cd <dir> && make`.

## Per-directory checklist

The order below puts simpler / smaller chapters first so each merge is
small and verifiable, and front-loads the `lib/` directory because it is
the only one that produces the static archive everything else links
against — fixing it early surfaces any header-ordering mistakes loudly.

- [ ] **`lib/`** (28 sources). Implementation files for `libapue.a`.
  Most include `apue.h` because they implement its prototypes.
  Sources of interest: `error.c` (varargs), `daemonize.c` (signals,
  resource limits), `ptyfork.c`/`ptyopen.c` (termios, ioctl conditional
  on `TIOCGWINSZ`), `signal.c`/`signalintr.c` (sigaction),
  `lockreg.c`/`locktest.c` (fcntl). `strerror.c` is in the directory
  but **not in the Makefile's `OBJS`** — leave its include block alone
  unless wired in.
- [ ] **`stdio/`** (6 sources). Pure stdio examples — small, safe
  warm-up.
- [ ] **`fileio/`** (5). `<fcntl.h>`, `<unistd.h>`, `<sys/stat.h>`.
- [ ] **`filedir/`** (10). Adds `<dirent.h>`, `<sys/stat.h>`,
  `<pwd.h>`/`<grp.h>` for some files.
- [ ] **`environ/`** (6). `<stdlib.h>`, `<setjmp.h>`,
  `<sys/resource.h>`.
- [ ] **`proc/`** (15). `<sys/wait.h>`, `<unistd.h>`, `<sys/times.h>`,
  `<sys/resource.h>`.
- [ ] **`signals/`** (12). `<signal.h>`, `<setjmp.h>`, `<unistd.h>`,
  `<sys/wait.h>`.
- [ ] **`relation/`** (1). Small.
- [ ] **`daemons/`** (3). `<syslog.h>`, `<fcntl.h>`,
  `<sys/resource.h>`, `<signal.h>`.
- [ ] **`threads/`** (12). `<pthread.h>`, `<time.h>`. Note the macOS
  carve-out — `timedlock.c` is excluded on macOS. Also note that
  `condvar.c`, `maketimeout.c`, `mutex1.c`, `mutex2.c`, `mutex3.c`,
  `rwlock.c` are compiled to `.o` but not linked into executables;
  their includes still need fixing.
- [ ] **`threadctl/`** (4). `<pthread.h>`.
- [ ] **`ipc1/`** (12). `<sys/ipc.h>`, `<sys/shm.h>`, `<sys/sem.h>`,
  `<sys/msg.h>`.
- [ ] **`ipc2/`** (top-level: `bindunix`, `pollmsg`, `recvfd2`,
  `sendfd2`, `sendmsg`). `<sys/socket.h>`, `<sys/un.h>`, `<poll.h>`.
- [ ] **`ipc2/open/`** + **`ipc2/opend/`** + **`ipc2/open.fe/`** +
  **`ipc2/opend.fe/`** — nested mini-projects with their own
  `Makefile`s and shared `*.h`. The headers (`open.h`, `opend.h`)
  declare extern variables of POSIX types, so they probably need
  `<sys/types.h>` themselves.
- [ ] **`sockets/`** (10). `<sys/socket.h>`, `<arpa/inet.h>`,
  `<netinet/in.h>`, `<netdb.h>`, `<sys/un.h>`.
- [ ] **`pty/`** (3 — `driver`, `loop`, `main`). `<termios.h>`,
  `<sys/ioctl.h>`, `<signal.h>`. Linked into one binary by the
  Makefile (multi-source executable).
- [ ] **`termios/`** (7). `<termios.h>`, `<sys/ioctl.h>`,
  `<unistd.h>`.
- [ ] **`advio/`** — already mostly done; double-check the entries on
  the TODO match what's actually in the directory after the
  conversion (`readn.c` and `writen.c` were *removed* in commit
  `2cdfa09`, replaced by `lib/readn.c` / `lib/writen.c`).
- [ ] **`db/`** (2 — `db.c`, `t4.c`). Plus `db.h` in the directory.
- [ ] **`datafiles/`** — small.
- [ ] **`exercises/`** (15). Mixed bag; treat each file individually.
- [ ] **`figlinks/`** — likely just symlinks for the book's figure
  numbering; verify nothing to change.
- [ ] **`printer/`** (3 — `print.c`, `printd.c`, `util.c`).
  `<sys/socket.h>`, threading, IPP-related includes.
- [ ] **`standards/`** — `conf.c.modified`, `makeconf.awk`,
  `makeopt.awk`. The `.modified` file gets `#include` adjustments;
  the awk scripts are out of scope.
- [ ] **`include/`** — see "Header sweep" below.

## Bug to fix as part of this work

`apue.3e/intro/getcputc.c` was converted in commit `2cdfa09` but the
new include block dropped `apue.h` even though the file calls
`err_sys()`. It compiles only by accident (implicit declaration warning,
linked because everything pulls in `libapue`). Add `#include "apue.h"`
back, plus `<sys/types.h>` ahead of it.

While converting other directories, watch for the same regression
class: any file calling `err_*`, `log_*`, `TELL_*`, `WAIT_*`, the
APUE lock macros, or `MAXLINE`/`FILE_MODE` must keep `apue.h`.

## Header sweep — `include/apue.h`

After the per-directory work but before deleting `apue2.h`:

1. Decide whether `apue.h` should pull in its own type dependencies
   (`<sys/types.h>` for `pid_t`/`off_t`/`ssize_t`,
   `<sys/stat.h>` for `S_I*` used in `FILE_MODE`/`DIR_MODE`,
   `<termios.h>` for `struct termios`, `<sys/ioctl.h>` /
   `<termios.h>` for `struct winsize`, `<signal.h>` for
   `Sigfunc`/`SIG_ERR`-using prototypes — actually `signal.h` is not
   strictly needed for `Sigfunc` since it's defined locally).
   - **Pedagogical preference**: keep `apue.h` declaration-only and
     require each TU to include the right type headers first. That
     reinforces the "see what you depend on" lesson.
   - **Pragmatic preference**: add the minimum set inside `apue.h`
     itself so users do not have to add `<sys/types.h>` to every
     file. Self-contained headers are a defensible idiom.
   - Pick one explicitly and document the choice at the top of
     `apue.h`.
2. If choice is "self-contained", drop the `/* needed for apue.h */`
   includes from every TU during/after the per-directory pass.

## Final cleanup (per existing TODO)

- [ ] In every converted file, move `#include "apue.h"` to the end of
  the include list (currently several files have it mid-list, e.g.
  `intro/shell1.c` puts `apue.h` before `<sys/wait.h>`).
- [ ] Run clang-format with an `IncludeBlocks: Regroup` /
  `SortIncludes: CaseSensitive` config and verify the trailing
  `/* for ... */` comments survive (clang-format preserves trailing
  comments by default; spot-check a few files).
- [ ] Delete `apue.3e/include/apue2.h`.
- [ ] Grep for any stragglers: `grep -rn 'apue2\.h' apue.3e` should be
  empty.
- [ ] Update `TODO.org` to mark item #1 done.

## Verification at each step

After each directory is converted:

```sh
cd /apue/apue.3e/<dir> && make clean && make 2>&1 | tee /tmp/build.log
```

Expect zero warnings beyond the platform's baseline. Note that
`-ansi -Wall` plus `-D_GNU_SOURCE` already produces some preexisting
warnings — capture a baseline build log from `master` before starting
so regressions are easy to spot.

Across-the-tree sanity check after the include sweep finishes:

```sh
cd /apue/apue.3e && make clean && make
```

## Open questions

- Should we standardize on `<errno.h>` being included even where
  `errno` is only touched indirectly via `err_sys()`? Current
  convention says no — `apue.h` callers don't need `<errno.h>`.
- Do we want to preserve the `#if defined(MACOS) || !defined(TIOCGWINSZ)`
  guard around `<sys/ioctl.h>` in the converted files, or simplify
  now that we only target Linux + macOS?
- A few headers in `apue.3e/` outside `include/` (e.g.
  `ipc2/open/open.h`, `printer/print.h`, `printer/ipp.h`) currently
  rely on `apue2.h` having been included first. Treat these as part
  of their owning directory's pass.
