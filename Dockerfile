FROM registry.fedoraproject.org/fedora:45


RUN --mount=type=cache,target=/var/cache/libdnf5 \
    --mount=type=cache,target=/var/lib/dnf \
    sed -i -e "s@tsflags=nodocs@#tsflags=nodocs@g" /etc/dnf/dnf.conf && \
    echo "keepcache=True" >> /etc/dnf/dnf.conf && \
    dnf upgrade -y && \
    dnf install -y --skip-unavailable \
                   autoconf  \
                   automake  \
                   clang  \
                   clang-tools-extra  \
                   cppcheck  \
                   g++  \
                   gcc  \
                   gdb  \
                   htop  \
                   lldb  \
                   libbsd \
                   libbsd-devel \
                   llvm \
                   man  \
                   man-db  \
                   man-pages  \
                   meson  \
                   nano  \
                   ninja-build  \
                   ps  \
                   tmux ;  \
    echo 'set debuginfod enabled off' > /root/.gdbinit

COPY musl /apue/musl/musl
RUN mkdir /apue/musl/bld && mkdir /apue/musl/bldInstall && \
    cd /apue/musl/bld && \
    CC=clang CFLAGS='-g -O0' ../musl/configure --prefix=/apue/musl/bldInstall && \
    make && make install

# The Emacs binary, only when USE_EMACS=1 (the Makefile default). Its packages are
# never installed here: `make shell` bind-mounts the vendored entrypoint/dotfiles/.emacs.d
# (with its committed elpa/), and init.el byte-compiles elpa/ on Emacs's first start.
# Placed after the musl build so toggling USE_EMACS never invalidates that layer, and
# before the apue.3e COPY so editing sources never re-runs this install.
ARG USE_EMACS=0
RUN --mount=type=cache,target=/var/cache/libdnf5 \
    --mount=type=cache,target=/var/lib/dnf \
    if [ "$USE_EMACS" = "1" ]; then \
        dnf install -y emacs ; \
    fi

COPY .clang-format /apue/
COPY entrypoint/dotfiles/.lldbinit /root/.lldbinit

RUN echo "source ~/.extrabashrc" >> ~/.bashrc

COPY apue.3e /apue/apue.3e/

# Build the examples at image-build time, OUT OF SOURCE to /apue/build (a sibling of
# apue.3e, NOT under apue.3e). The `make shell` bind-mount covers /apue/apue.3e only,
# so a build written inside it (apue.3e/build) would be hidden by the mount; /apue/build
# sits outside the mount and survives, giving Emacs/clangd a compile_commands.json with
# no prior `make build` (init.el points --compile-commands-dir here). compile + test make
# this a build gate too, the way gltron runs ctest at image-build. `make build` still
# writes apue.3e/build against the live mounted tree; this baked copy is clangd's default.
RUN cd /apue/apue.3e && \
    meson setup /apue/build --native-file native-linux.ini && \
    meson compile -C /apue/build && \
    meson test -C /apue/build --print-errorlogs

# No ENTRYPOINT: every `make` target sets --entrypoint /bin/bash
# explicitly. The previous /entrypoint.sh was stale (referenced a
# different project's docs build) and was never copied into the image
# anyway, so docker would have errored on container start without
# the --entrypoint override.
