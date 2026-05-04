project('apue', 'c',
  version : '3e',
  meson_version : '>= 0.60.0',
  default_options : [
    # c99 (vs the make build's -ansi == c89) so that falling off the
    # end of main() returns 0 — the original textbook hello1.c relies
    # on this. C99 also matches the POSIX.1-2008 feature test macros
    # (_POSIX_C_SOURCE 200809L) that apue.h sets.
    'c_std=c99',
    'warning_level=1',
    'buildtype=debug',
  ],
)

cc = meson.get_compiler('c')
host_os = host_machine.system()

if host_os == 'linux'
  linux_args = ['-DLINUX', '-D_GNU_SOURCE']
  # -Wno-unused-command-line-argument silences the noisy linker-flag
  # warnings the musl-clang wrapper emits on every compile-only run.
  # Only clang accepts this flag; gcc warns about it, so guard with
  # has_argument().
  if cc.has_argument('-Wno-unused-command-line-argument')
    linux_args += ['-Wno-unused-command-line-argument']
  endif
  add_project_arguments(linux_args, language : 'c')
elif host_os == 'darwin'
  add_project_arguments('-DMACOS', '-D_DARWIN_C_SOURCE',
                        language : 'c')
else
  error('apue meson build supports linux and darwin only; got: ' + host_os)
endif

apue_inc = include_directories('include')

threads_dep = dependency('threads')
rt_dep = (host_os == 'linux'
          ? cc.find_library('rt', required : true)
          : dependency('', required : false))

subdir('include')
subdir('lib')
subdir('sockets')   # before printer, which links a helper from here

subdir('intro')
subdir('stdio')
subdir('fileio')
subdir('filedir')
subdir('environ')
subdir('proc')
subdir('signals')
subdir('relation')
subdir('daemons')
subdir('threads')
subdir('threadctl')
subdir('ipc1')
subdir('ipc2')
subdir('pty')
subdir('termios')
subdir('advio')
subdir('db')
subdir('datafiles')
subdir('exercises')
subdir('printer')
subdir('standards')
# figlinks/ is all symlinks to other example sources — nothing to build.
