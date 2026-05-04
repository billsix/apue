threads_extra = [libapue_dep, threads_dep]
if host_os == 'linux'
  threads_extra += [rt_dep]
endif

base_progs = ['badexit2', 'cleanup', 'exitstatus', 'threadid']
threads_exes = {}
foreach p : base_progs
  threads_exes += {p: executable(p, p + '.c', dependencies : threads_extra)}
endforeach

# Smoke tests — threadid + exitstatus both exercise pthread_create
# and exit cleanly within ~1 second. badexit2 uses pthread_exit from
# main without joining (intentional bug demo); cleanup tests cleanup
# handlers — both are noisy and skipped here.
test('threads_threadid',   threads_exes['threadid'],   timeout : 10)
test('threads_exitstatus', threads_exes['exitstatus'], timeout : 10)

# timedlock uses pthread_mutex_timedlock — not available on macOS.
if host_os != 'darwin'
  executable('timedlock', 'timedlock.c', dependencies : threads_extra)
endif

# barrier is Linux-only in the make build (pthread_barrier_*).
if host_os == 'linux'
  executable('barrier', 'barrier.c', dependencies : threads_extra)
endif

# condvar/maketimeout/mutex1-3/rwlock are compile-only in the make build.
static_library('threads_examples',
  files(
    'condvar.c',
    'maketimeout.c',
    'mutex1.c',
    'mutex2.c',
    'mutex3.c',
    'rwlock.c',
  ),
  dependencies : threads_extra,
  build_by_default : true,
)
