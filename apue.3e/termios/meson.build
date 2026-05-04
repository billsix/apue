progs = [
  'csize',
  'settty',
  't_getpass',
  't_isatty',
  't_raw',
  't_ttyname',
  'winch',
]

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# ctermid.o, getpass.o, isatty.o, ttyname.o compile-only in make build.
static_library('termios_libc_reimpls',
  files('ctermid.c', 'getpass.c', 'isatty.c', 'ttyname.c'),
  dependencies : libapue_dep,
  build_by_default : true,
)
