progs = ['atfork', 'suspend']

foreach p : progs
  executable(p, p + '.c', dependencies : [libapue_dep, threads_dep])
endforeach

# detach.o, getenv1.o, getenv2.o, getenv3.o compile-only in the make build.
static_library('threadctl_examples',
  files('detach.c', 'getenv1.c', 'getenv2.c', 'getenv3.c'),
  dependencies : [libapue_dep, threads_dep],
  build_by_default : true,
)

# timeout.o is Linux-only in the make build.
if host_os == 'linux'
  static_library('threadctl_timeout', 'timeout.c',
    dependencies : [libapue_dep, threads_dep],
    build_by_default : true,
  )
endif
