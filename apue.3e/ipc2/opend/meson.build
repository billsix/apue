opend_common = files('main.c', 'request.c', 'cliargs.c', 'client.c')

executable('opend.poll',
  opend_common + files('loop.poll.c'),
  dependencies : libapue_dep,
)
executable('opend.select',
  opend_common + files('loop.select.c'),
  dependencies : libapue_dep,
)
