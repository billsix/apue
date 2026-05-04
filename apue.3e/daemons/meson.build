# The make build only compiles these to .o files (no executables) so the
# textbook can show daemonize/reread/single side-by-side without linking.
# Mirror that by collecting them into a build-only static_library.

static_library('daemons_examples',
  files('init.c', 'reread.c', 'reread2.c', 'single.c'),
  dependencies : libapue_dep,
  build_by_default : true,
)
