# libapue.a — implementation of the helpers declared in include/apue.h.
# Source list mirrors lib/Makefile's $(OBJS).
# Files in this directory NOT in this list (Orecvfd.c, nspipe.c, semaph.c,
# strerror.c, sleep.c) are intentionally excluded — same as the make build.

libapue_sources = files(
  'bufargs.c',
  'cliconn.c',
  'clrfl.c',
  'daemonize.c',
  'error.c',
  'errorlog.c',
  'lockreg.c',
  'locktest.c',
  'openmax.c',
  'pathalloc.c',
  'popen.c',
  'prexit.c',
  'prmask.c',
  'ptyfork.c',
  'ptyopen.c',
  'readn.c',
  'recvfd.c',
  'senderr.c',
  'sendfd.c',
  'servaccept.c',
  'servlisten.c',
  'setfd.c',
  'setfl.c',
  'signal.c',
  'signalintr.c',
  'sleepus.c',
  'spipe.c',
  'tellwait.c',
  'ttymodes.c',
  'writen.c',
)

libapue = static_library('apue', libapue_sources,
  include_directories : apue_inc,
)

libapue_dep = declare_dependency(
  link_with : libapue,
  include_directories : apue_inc,
)
