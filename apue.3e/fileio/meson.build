progs = ['fileflags', 'hole', 'mycat', 'seek']

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# setfl.c is compile-only in the make build. Express it as a tiny
# static_library so meson actually compiles it for warnings coverage.
static_library('fileio_setfl_check', 'setfl.c',
  dependencies : libapue_dep,
  build_by_default : true,
)
