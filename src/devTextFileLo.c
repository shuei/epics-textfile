// -*- coding: utf-8; mode: c; c-basic-offiset: 4 -*-

//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 High Energy Accelerator Research Organization (KEK)
//
// text file Device Support 0.0.0
// and higher are distributed subject to a Software License Agreement found
// in file LICENSE that is included with this distribution.
//
// Author: Shuei Yamada (shuei@post.kek.jp)
//
//////////////////////////////////////////////////////////////////////////

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

//
#include "longoutRecord.h"
#include "cantProceed.h"
#include "dbAccess.h"
#include "devSup.h"
#include "alarm.h"
#include "errlog.h"
#include "recGbl.h"
#include "link.h"
#include "epicsExport.h"
#include "epicsTime.h"

//
#include "devTextFile.h"

//
static int devTextFileLoDebug = 0;

/***************************************************************
 * longout (command/response IO)
 ***************************************************************/
static long init(void);
static long init_record(struct longoutRecord *);
static long write_lo(struct longoutRecord *);

struct {
    long        number;
    DEVSUPFUN   report;
    DEVSUPFUN   init;
    DEVSUPFUN   init_record;
    DEVSUPFUN   get_ioint_info;
    DEVSUPFUN   write_lo;
    DEVSUPFUN   special_linconv;
} devTextFileLo = {
    6,
    NULL,
    init,
    init_record,
    NULL,
    write_lo,
    NULL
};

epicsExportAddress(dset, devTextFileLo);

//
static long init(void)
{
    return 0;
}

//
static long init_record(struct longoutRecord *prec)
{
    DBLINK *plink = &prec->out;

    //
    if (devTextFileLoDebug > 0) {
        printf("%s (devTextFileLo): out=%s\n", prec->name, plink->value.instio.string);
    }

    // Link type must be INST_IO
    if (plink->type != INST_IO) {
        errlogPrintf("%s (devTextFileLo): address type must be \"INST_IO\"\n", prec->name);
        prec->pact = 1;
        return -1;
    }

    // Allocate private data storage area
    TextFile_t *dpvt = callocMustSucceed(1, sizeof(TextFile_t), "calloc for private_t failed ");
    prec->dpvt = dpvt;

    // Extract output filename
    const char *pstr = plink->value.instio.string;

    // check if read flag is specified in OUT field
    if (pstr[0] == '<') {
        dpvt->flag = kRead;
        pstr++;
    }

    const size_t fsize = strlen(pstr) + 1;
    //if (fsize > MAX_INSTIO_STRING) {
    //    errlogPrintf("%s (devTextFileLo): INP field is too long\n", prec->name);
    //    return -1;
    //}
    dpvt->name = callocMustSucceed(1, fsize, "calloc for filename failed");
    strcpy(dpvt->name, pstr);

    //
    if (dpvt->flag == kRead) {
        const char *filename = pstr;

        //
        long ret = devTextFileRead(filename, &prec->val, (dbCommon *)prec, DBF_LONG, 1, devTextFileLoDebug);

        //
        if (ret < 0) {
            return -1;
        }
    }

    //
    return 0;
}

//
static long write_lo(struct longoutRecord *prec)
{
    //DBLINK *plink = &prec->inp;
    TextFile_t *dpvt = prec->dpvt;
    const char *filename = dpvt->name;

    //
    if (devTextFileLoDebug > 0) {
        printf("%s (devTextFileLo): filename: %s\n", prec->name, filename);
    }

    //
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
        errlogPrintf("%s (devTextFileLo): can't open \"%s\" for writing: %s\n", prec->name, filename, errmsg);
        prec->nsev = INVALID_ALARM;
        prec->nsta = WRITE_ACCESS_ALARM;
        return -1;
    }

    //
    int retval = 0;

    // timestamp
    char datetime[128];
    epicsTimeToStrftime(datetime, sizeof(datetime), "%Y-%m-%d %T", &prec->time);

    char wday[32];
    epicsTimeToStrftime(wday, sizeof(wday), "(%a)", &prec->time);

    // hostname
    // gethostname() won't work if /etc/hostname is empty
    struct utsname buf;
    uname(&buf);

    //
    if (devTextFileLoDebug > 0) {
        printf("%s (devTextFileLo): %s.%06d %s\n", prec->name, datetime, prec->time.nsec/1000, wday);
    }

    //
    const int32_t val = prec->val;
    const int ret = fprintf(fp,
                            "# saved by devTextFileLo on %s\n# %s as of %s.%06d %s\n%d\n",
                            buf.nodename,
                            prec->name, datetime, prec->time.nsec/1000, wday,
                            val);

    if (ret < 0) {
        // write error
        errlogPrintf("%s (devTextFileLo): No data was written to the file: \"%s\"\n", prec->name, filename);
        prec->nsev = INVALID_ALARM;
        prec->nsta = WRITE_ALARM;
        retval = -1;
    }

    //
    prec->udf = FALSE;

    // cleanup
    fclose(fp);
    fp = NULL;

    //
    return retval;
}

// Register symbol(s) used by IOC core
epicsExportAddress(int, devTextFileLoDebug);

// end
