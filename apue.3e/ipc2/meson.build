ipc2_extra = [libapue_dep, threads_dep]

progs = ['bindunix', 'pollmsg', 'sendmsg']
foreach p : progs
  executable(p, p + '.c', dependencies : ipc2_extra)
endforeach

# Linux-only fd-passing helper objects in the make build.
if host_os == 'linux'
  static_library('ipc2_fdpass',
    files('recvfd2.c', 'sendfd2.c'),
    dependencies : ipc2_extra,
    build_by_default : true,
  )
endif

subdir('open')
subdir('opend')
subdir('open.fe')
subdir('opend.fe')
