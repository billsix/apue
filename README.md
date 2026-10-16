# apue

Example code from *Advanced Programming in the UNIX Environment*, 3rd edition
(W. Richard Stevens & Stephen A. Rago), re-tooled for a clean, debuggable,
containerized build. The ~220 example programs live under `apue.3e/`.

This fork:

- replaces the original recursive Make with **Meson + Ninja**,
- builds against a **debug musl-clang** libc (compiled from source) on Linux,
- ships a Fedora podman dev container with gdb/lldb ready to go, and
- **modernizes the examples to modern C (C23)** rather than keeping them
  byte-faithful to the book — runtime behavior is preserved, and the tree
  compiles warning-clean. See `tasks/archive/2026/10/16/modernize-to-modern-c.md`.

## Quick start

Requires `podman` and `make`.

```sh
make image     # build the image (compiles musl-clang from source — slow first time)
make shell     # ephemeral container shell, mounted at /apue
make build     # configure + compile + test apue.3e via Meson, inside the container
```

Inside the shell the Meson build lives in `apue.3e/build/`. lldb/gdb work — the
container starts with ptrace capability and ASLR disabled.

## Layout

| Path | What |
| --- | --- |
| `apue.3e/` | The textbook example tree (chapters + `lib/libapue.a` + `include/apue.h`) |
| `musl/` | Vendored musl, built debuggable into the image |
| `entrypoint/` | Container shell + dotfiles |
| `tasks/` | Active work; `tasks/archive/<YYYY>/<MM>/<DD>/` holds completed work, including the modernization design notes |

`make help` lists the targets.

## Origin

Example code is from apuebook.com (Stevens & Rago). See `apue.3e/README` and
`apue.3e/DISCLAIMER`.
