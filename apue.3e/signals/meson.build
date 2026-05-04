progs = [
  'critical',
  'mask',
  'read1',
  'read2',
  'reenter',
  'sigtstp',
  'sigusr',
  'suspend1',
  'suspend2',
]

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# Multi-source executables.
executable('tsleep2',
  files('tsleep2.c', 'sleep2.c'),
  dependencies : libapue_dep,
)
executable('systest2',
  files('systest2.c', 'system.c'),
  dependencies : libapue_dep,
)

# abort.o, sleep1.o, sleep2.o, system.o compile-only in the make build.
static_library('signals_examples',
  files('abort.c', 'sleep1.c', 'sleep2.c', 'system.c'),
  dependencies : libapue_dep,
  build_by_default : true,
)

# child is Linux-only in the make build.
if host_os == 'linux'
  executable('child', 'child.c', dependencies : libapue_dep)
endif
