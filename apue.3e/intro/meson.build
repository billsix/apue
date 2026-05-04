progs = [
  'getcputc',
  'hello',
  'ls1',
  'mycat',
  'shell1',
  'shell2',
  'testerror',
  'uidgid',
]

intro_exes = {}
foreach p : progs
  intro_exes += {p: executable(p, p + '.c', dependencies : libapue_dep)}
endforeach

# Smoke tests — only programs that take no stdin and need no TTY/network.
test('intro_hello',     intro_exes['hello'])
test('intro_uidgid',    intro_exes['uidgid'])
test('intro_ls1',       intro_exes['ls1'], args : ['.'])
test('intro_testerror', intro_exes['testerror'])
