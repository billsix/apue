FROM registry.fedoraproject.org/fedora:44


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
                   emacs  \
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

COPY .clang-format /apue/
COPY entrypoint/dotfiles/.lldbinit /root/.lldbinit

RUN echo "source ~/.extrabashrc" >> ~/.bashrc

COPY apue.3e /apue/apue.3e/

# No ENTRYPOINT: every `make` target sets --entrypoint /bin/bash
# explicitly. The previous /entrypoint.sh was stale (referenced a
# different project's docs build) and was never copied into the image
# anyway, so docker would have errored on container start without
# the --entrypoint override.
