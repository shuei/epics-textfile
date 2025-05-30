#ifndef DEVTEXTFILE
#define DEVTEXTFILE

//
#include <dbScan.h>

//
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_INSTIO_STRING  256
#define ERRBUF 1024

typedef struct {
    IOSCANPVT    ioscanpvt;
    char        *name;
    char         errmsg[ERRBUF];
} TextFile_t;

#endif
