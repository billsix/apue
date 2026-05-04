# `print` and `printd` link `util.c` plus helpers from sockets/.
# The make build reaches across directories for $(ROOT)/sockets/clconn2.o
# and initsrv2.o; here we consume socket_helpers_dep, declared in
# sockets/meson.build and visible because sockets/ is processed before
# this directory at the top level.

printer_extra = [libapue_dep, socket_helpers_dep, threads_dep]

executable('print',
  files('print.c', 'util.c'),
  dependencies : printer_extra,
)

executable('printd',
  files('printd.c', 'util.c'),
  dependencies : printer_extra,
)
