progs = [
  'add2',
  'add2stdio',
  'devzero',
  'myuclc',
  'pipe1',
  'pipe2',
  'pipe4',
  'popen1',
  'popen2',
  'tshm',
]

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# popen.o, slock.o, tellwait.o are compile-only in the make build.
static_library('ipc1_examples',
  files('popen.c', 'slock.c', 'tellwait.c'),
  dependencies : libapue_dep,
  build_by_default : true,
)
