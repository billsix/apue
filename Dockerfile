FROM registry.fedoraproject.org/fedora:43


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
                   lldb  \
                   libbsd \
                   libbsd-devel \
                   man  \
                   man-db  \
                   man-pages  \
                   nano  \
                   tmux ;  \
    echo 'set debuginfod enabled off' > /root/.gdbinit

COPY apue.3e /apue/apue.3e/
COPY musl /apue/musl/

COPY .clang-format /apue/

RUN echo "source ~/.extrabashrc" >> ~/.bashrc

ENTRYPOINT ["/entrypoint.sh"]
