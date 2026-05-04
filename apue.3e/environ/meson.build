progs = ['doatexit', 'echoarg', 'getrlimit', 'hello1', 'testjmp']

environ_exes = {}
foreach p : progs
  environ_exes += {p: executable(p, p + '.c', dependencies : libapue_dep)}
endforeach

# Smoke tests.
test('environ_hello1',    environ_exes['hello1'])
test('environ_getrlimit', environ_exes['getrlimit'])

# opendata.o is compile-only in the make build.
static_library('environ_opendata_check', 'opendata.c',
  dependencies : libapue_dep,
  build_by_default : true,
)

# scope.c is a textbook fragment with no #include directives and is
# not intended to compile standalone. The make build still tries to
# compile it (`scope.o`); we skip it here since meson's static_library
# would error on a TU that produces no symbols. If you want to mirror
# the make build's compile attempt, add scope.c above.
