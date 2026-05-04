progs = [
  'bo',
  'fifo1',
  'getlogin',
  'goodexit',
  'longpath',
  'pendlock',
  'prtime',
  'sizepipe',
  'vfork3',
  'zombie',
]

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# Compile-only sources from the make build's `all:` target.
static_library('exercises_examples',
  files(
    'asyncsocket.c',
    'openmax.c',
    'sleep.c',
    'sleepus_poll.c',
    'sleepus_select.c',
  ),
  dependencies : libapue_dep,
  build_by_default : true,
)

# Platform-specific getpw variant.
if host_os == 'linux'
  executable('getpwsvr4', 'getpwsvr4.c', dependencies : libapue_dep)
elif host_os == 'darwin'
  executable('getpw44bsd', 'getpw44bsd.c', dependencies : libapue_dep)
endif

# fmemopen.c — uses BSD-only funopen(); built on macOS only in the make
# build. The agent flagged that this file does not compile on Linux.
if host_os == 'darwin'
  static_library('exercises_fmemopen', 'fmemopen.c',
    dependencies : libapue_dep,
    build_by_default : true,
  )
endif
