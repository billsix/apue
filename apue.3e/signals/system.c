#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

int system(const char *cmdstring) /* with appropriate signal handling */
{
    if (cmdstring == nullptr) {
        return (1); /* always a command processor with UNIX */
    }

    struct sigaction ignore;
    ignore.sa_handler = SIG_IGN; /* ignore SIGINT and SIGQUIT */
    sigemptyset(&ignore.sa_mask);
    ignore.sa_flags = 0;
    struct sigaction saveintr;
    if (sigaction(SIGINT, &ignore, &saveintr) < 0) {
        return (-1);
    }
    struct sigaction savequit;
    if (sigaction(SIGQUIT, &ignore, &savequit) < 0) {
        return (-1);
    }
    sigset_t chldmask;
    sigemptyset(&chldmask); /* now block SIGCHLD */
    sigaddset(&chldmask, SIGCHLD);
    sigset_t savemask;
    if (sigprocmask(SIG_BLOCK, &chldmask, &savemask) < 0) {
        return (-1);
    }

    int status;
    const pid_t pid = fork();
    if (pid < 0) {
        status = -1;       /* probably out of processes */
    } else if (pid == 0) { /* child */
        /* restore previous signal actions & reset signal mask */
        sigaction(SIGINT, &saveintr, nullptr);
        sigaction(SIGQUIT, &savequit, nullptr);
        sigprocmask(SIG_SETMASK, &savemask, nullptr);

        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        _exit(127); /* exec error */
    } else {        /* parent */
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1; /* error other than EINTR from waitpid() */
                break;
            }
        }
    }

    /* restore previous signal actions & reset signal mask */
    if (sigaction(SIGINT, &saveintr, nullptr) < 0) {
        return (-1);
    }
    if (sigaction(SIGQUIT, &savequit, nullptr) < 0) {
        return (-1);
    }
    if (sigprocmask(SIG_SETMASK, &savemask, nullptr) < 0) {
        return (-1);
    }

    return (status);
}
