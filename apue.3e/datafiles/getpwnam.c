#include <pwd.h>
#include <stddef.h>
#include <string.h>

struct passwd *getpwnam(const char *name) {
    struct passwd *ptr;

    setpwent();
    for (;;) {
        ptr = getpwent();
        if (ptr == nullptr) {
            break;
        }
        if (strcmp(name, ptr->pw_name) == 0) {
            break; /* found a match */
        }
    }
    endpwent();
    return (ptr); /* ptr is NULL if no match found */
}
