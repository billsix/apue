#include <stdlib.h>     /* for exit */
#include <fcntl.h>      /* for O_RDWR, O_CREAT, O_TRUNC */
#include <sys/types.h>  /* needed for apue.h */
#include <sys/stat.h>   /* for FILE_MODE constants (S_I*) */

#include "apue.h"
#include "apue_db.h"

int main(void) {
    DBHANDLE db;

    if ((db = db_open("db4", O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) == NULL) {
        err_sys("db_open error");
    }

    if (db_store(db, "Alpha", "data1", DB_INSERT) != 0) {
        err_quit("db_store error for alpha");
    }
    if (db_store(db, "beta", "Data for beta", DB_INSERT) != 0) {
        err_quit("db_store error for beta");
    }
    if (db_store(db, "gamma", "record3", DB_INSERT) != 0) {
        err_quit("db_store error for gamma");
    }

    db_close(db);
    exit(0);
}
