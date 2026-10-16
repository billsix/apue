#!/bin/env bash

# Format the apue.3e example tree ONLY. musl/ is vendored upstream libc (built
# into the image, never mounted into this container) and is never reformatted.
# build/ is the generated meson out-of-tree build directory. Runs in the
# container (apue.3e is mounted at /apue/apue.3e) or on the host from the repo
# root (where the tree is ./apue.3e).

if [ -d /apue/apue.3e ]; then
  cd /apue/apue.3e
elif [ -d apue.3e ]; then
  cd apue.3e
fi

find . -path ./build -prune -o \
  \( -iname "*.c" -o -iname "*.h" \) -print0 | xargs -0 clang-format -i
