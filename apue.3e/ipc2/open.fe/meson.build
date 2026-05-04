executable('openclient_fe',
  files('main.c', 'open.c'),
  dependencies : libapue_dep,
)
# Renamed in meson namespace from 'openclient' (which exists in ipc2/open/)
# to avoid a target-name collision; the make build allowed both because
# they live in separate per-directory builds.
