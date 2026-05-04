# Helper sources used by multiple executables here, and by printer/.
# Expose them via dependencies so printer/ can link without reaching
# across the source tree for .o files (the make build does
# `print: print.o util.o $(ROOT)/sockets/clconn2.o`).

socket_helpers_lib = static_library('socket_helpers',
  files('clconn.c', 'clconn2.c', 'initsrv1.c', 'initsrv2.c'),
  dependencies : libapue_dep,
)

socket_helpers_dep = declare_dependency(link_with : socket_helpers_lib)

# Programs.
executable('findsvc', 'findsvc.c', dependencies : libapue_dep)

executable('ruptime',
  files('ruptime.c', 'clconn2.c'),
  dependencies : libapue_dep,
)
executable('ruptime-dg',
  files('ruptime-dg.c', 'clconn2.c'),
  dependencies : libapue_dep,
)
executable('ruptimed',
  files('ruptimed.c', 'initsrv2.c'),
  dependencies : libapue_dep,
)
executable('ruptimed-fd',
  files('ruptimed-fd.c', 'initsrv2.c'),
  dependencies : libapue_dep,
)
executable('ruptimed-dg',
  files('ruptimed-dg.c', 'initsrv2.c'),
  dependencies : libapue_dep,
)
