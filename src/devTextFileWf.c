// -*- coding: utf-8; mode: c; c-basic-offiset: 4 -*-

//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 High Energy Accelerator Research Organization (KEK)
//
// text file Device Support 0.0.0
// and higher are distributed subject to a Software License Agreement found
// in file LICENSE that is included with this distribution.
//
// Current Author: Shuei Yamada (shuei@post.kek.jp)
// Original Author: Jun-ichi Odagiri
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
#include <unistd.h>

//
#include "waveformRecord.h"
#include "cantProceed.h"
#include "dbAccess.h"
#include "devSup.h"
#include "alarm.h"
#include "errlog.h"
#include "recGbl.h"
#include "link.h"
#include "epicsExport.h"

//
#include "devTextFile.h"

//
static int devTextFileWfDebug = 0;

/***************************************************************
 * waveform (command/response IO)
 ***************************************************************/
static long init(void);
static long init_record(struct waveformRecord *);
static long read_wf(struct waveformRecord *);

struct {
    long        number;
    DEVSUPFUN   report;
    DEVSUPFUN   init;
    DEVSUPFUN   init_record;
    DEVSUPFUN   get_ioint_info;
    DEVSUPFUN   read_wf;
    DEVSUPFUN   special_linconv;
} devTextFileWf = {
    6,
    NULL,
    init,
    init_record,
    NULL,
    read_wf,
    NULL
};

epicsExportAddress(dset, devTextFileWf);

//
static long init(void)
{
    return 0;
}

//
static long init_record(struct waveformRecord *prec)
{
    DBLINK *plink = &prec->inp;

    //
    if (devTextFileWfDebug>0) {
        printf("%s (devTextFileWf): inp=%s nelm=%d\n", prec->name, plink->value.instio.string, prec->nelm);
    }

    // Link type must be INST_IO
    if (plink->type != INST_IO) {
        errlogPrintf("%s (devTextFileWf): address type must be \"INST_IO\"\n", prec->name);
        prec->pact = 1;
        return -1;
    }

    // Check FTVL field
    switch (prec->ftvl) {
    case DBF_CHAR:
    case DBF_UCHAR:
    case DBF_SHORT:
    case DBF_USHORT:
    case DBF_LONG:
    case DBF_ULONG:
    case DBF_FLOAT:
    case DBF_DOUBLE:
        break;
    default:
        errlogPrintf("%s (devTextFileWf): unsuppoted FTVL\n", prec->name);
        prec->pact = 1;
        return -1;
    }

    // Allocate private data storage area
    TextFile_t *dpvt = callocMustSucceed(1, sizeof(TextFile_t), "calloc for private_t failed ");
    prec->dpvt = dpvt;

    // Extract input filename
    const char *pstr = plink->value.instio.string;

    // check if read flag is specified in INP field
    if (pstr[0] == '<') {
        dpvt->flag = kRead;
        pstr++;
    }

    const size_t fsize = strlen(pstr) + 1;
    //if (fsize > MAX_INSTIO_STRING) {
    //    errlogPrintf("%s (devTextFileWf): INP field is too long\n", prec->name);
    //    return -1;
    //}
    dpvt->name = callocMustSucceed(1, fsize, "calloc for filename failed");
    strcpy(dpvt->name, pstr);

    //
    if (dpvt->flag == kRead) {
        const char *filename = pstr;

        //
        long ret = devTextFileRead(filename, prec->bptr, (dbCommon *)prec, prec->ftvl, prec->nelm, devTextFileWfDebug);

        //
        if (ret < 0) {
            prec->nord = 0;
            return -1;
        }

        //
        prec->nord = ret;
    }

    //
    return 0;
}

//
static long read_wf(struct waveformRecord *prec)
{
    //DBLINK *plink = &prec->inp;
    TextFile_t *dpvt = prec->dpvt;
    const char *filename = dpvt->name;

    //
    if (devTextFileWfDebug>0) {
        printf("%s (devTextFileWf): filename: %s, nelm:%d\n", prec->name, filename, prec->nelm);
    }

    //
    long ret = devTextFileRead(filename, prec->bptr, (dbCommon *)prec, prec->ftvl, prec->nelm, devTextFileWfDebug);

    //
    if (ret < 0) {
        prec->nord = 0;
        return -1;
    }

    //
    prec->nord = ret;

    //
    return 0;
}

// Register symbol(s) used by IOC core
epicsExportAddress(int, devTextFileWfDebug);

// end
