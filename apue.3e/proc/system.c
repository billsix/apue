#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

int system(const char *cmdstring) /* version without signal handling */
{
    if (cmdstring == nullptr) {
        return (1); /* always a command processor with UNIX */
    }

    int status;
    const pid_t pid = fork();
    if (pid < 0) {
        status = -1;       /* probably out of processes */
    } else if (pid == 0) { /* child */
        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        _exit(127); /* execl error */
    } else {        /* parent */
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1; /* error other than EINTR from waitpid() */
                break;
            }
        }
    }

    return (status);
}
