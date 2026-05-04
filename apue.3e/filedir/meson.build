progs = [
  'access',
  'cdpwd',
  'changemod',
  'devrdev',
  'filetype',
  'mycd',
  'umask',
  'unlink',
  'ftw8',
]

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# zap is Linux-only in the make build.
if host_os == 'linux'
  executable('zap', 'zap.c', dependencies : libapue_dep)
endif
