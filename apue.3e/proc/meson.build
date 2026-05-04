progs = [
  'echoall',
  'exec1',
  'exec2',
  'fork1',
  'fork2',
  'nice',
  'pruids',
  'tellwait1',
  'tellwait2',
  'test1',
  'times1',
  'vfork1',
  'wait1',
]

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# systest1 / systest3 link the local system.c override (a wrapper that
# replaces libc's system() so the test programs exercise APUE's version).
executable('systest1',
  files('systest1.c', 'system.c'),
  dependencies : libapue_dep,
)
executable('systest3',
  files('systest3.c', 'system.c'),
  dependencies : libapue_dep,
)

# system.o on its own is also compiled in the make build.
static_library('proc_system_check', 'system.c',
  dependencies : libapue_dep,
  build_by_default : true,
)

# pracct uses different defines per platform.
# Linux + macOS both want -DHAS_AXSIG -DHAS_ACORE.
executable('pracct', 'pracct.c',
  c_args : ['-DHAS_AXSIG', '-DHAS_ACORE'],
  dependencies : libapue_dep,
)
