executable('strftime', 'strftime.c', dependencies : libapue_dep)

# getpwnam.o is compile-only in the make build.
static_library('datafiles_getpwnam_check', 'getpwnam.c',
  dependencies : libapue_dep,
  build_by_default : true,
)
