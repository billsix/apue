# buf.c is intentionally excluded — it pokes at glibc FILE struct
# internals that no longer exist on current glibc, same as the make build
# (which has it commented out in PROGS).

progs = [
  'fgetsfputs',
  'getcharbug',
  'getcputc',
  'mkstemp',
  'tempfiles',
]

foreach p : progs
  executable(p, p + '.c', dependencies : libapue_dep)
endforeach

# memstr is Linux-only (uses memstream APIs).
if host_os == 'linux'
  executable('memstr', 'memstr.c', dependencies : libapue_dep)
endif
