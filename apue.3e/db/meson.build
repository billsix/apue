# The make build produces:
#   - libapue_db.so.1 (shared) linking against libapue
#   - libapue_db.a (static) — same source, archived
#   - t4 (the example client) linking against libapue_db (shared)
#
# Meson handles the dual static/shared from a single shared_library +
# static_library declaration. We use the shared lib for t4 (matching the
# make build) and set rpath to '$ORIGIN' so t4 can find the .so when
# run from the build directory.

apue_db_shared = shared_library('apue_db', 'db.c',
  soversion : '1',
  dependencies : libapue_dep,
)

apue_db_static = static_library('apue_db_static', 'db.c',
  dependencies : libapue_dep,
)

executable('t4', 't4.c',
  link_with : apue_db_shared,
  dependencies : libapue_dep,
  install_rpath : '$ORIGIN',
  build_rpath : meson.current_build_dir(),
)
