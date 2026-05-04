executable('opend_fe',
  files('main.c', 'cliargs.c', 'request.c'),
  dependencies : libapue_dep,
)
# Renamed from 'opend' (per the make build) to disambiguate from the
# opend.poll/opend.select pair in ipc2/opend/. Meson requires unique
# target names project-wide.
