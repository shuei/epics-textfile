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
#include <unistd.h>

//
#include "aiRecord.h"
#include "cantProceed.h"
#include "dbAccess.h"
#include "devSup.h"
#include "alarm.h"
#include "errlog.h"
#include "recGbl.h"
#include "link.h"
#include "epicsExport.h"
#include "epicsMath.h"

//
#include "devTextFile.h"

//
static int devTextFileAiDebug = 0;

/***************************************************************
 * ai (command/response IO)
 ***************************************************************/
static long init(void);
static long init_record(struct aiRecord *);
static long read_ai(struct aiRecord *);

struct {
    long        number;
    DEVSUPFUN   report;
    DEVSUPFUN   init;
    DEVSUPFUN   init_record;
    DEVSUPFUN   get_ioint_info;
    DEVSUPFUN   read_ai;
    DEVSUPFUN   special_linconv;
} devTextFileAi = {
    6,
    NULL,
    init,
    init_record,
    NULL,
    read_ai,
    NULL
};

epicsExportAddress(dset, devTextFileAi);

//
static long init(void)
{
    return 0;
}

//
static long init_record(struct aiRecord *prec)
{
    DBLINK *plink = &prec->inp;

    //
    if (devTextFileAiDebug>0) {
        printf("%s (devTextFileAi): inp=%s\n", prec->name, plink->value.instio.string);
    }

    // Link type must be INST_IO
    if (plink->type != INST_IO) {
        errlogPrintf("%s (devTextFileAi): address type must be \"INST_IO\"\n", prec->name);
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
    //    errlogPrintf("%s (devTextFileLi): INP field is too long\n", prec->name);
    //    return -1;
    //}
    dpvt->name = callocMustSucceed(1, fsize, "calloc for filename failed");
    strcpy(dpvt->name, pstr);

    //
    if (dpvt->flag == kRead) {
        const char *filename = pstr;
        double val = 0;

        //
        long ret = devTextFileRead(filename, &val, (dbCommon *)prec, DBF_DOUBLE, 1, devTextFileAiDebug);

        //
        if (ret < 0) {
            return -1;
        }

        // Apply ASLO & AOFF
        if (prec->aslo != 0.0) {
            val *= prec->aslo;
        }
        val += prec->aoff;

        //
        prec->val = val;
    }

    //
    return 0;
}

//
static long read_ai(struct aiRecord *prec)
{
    //DBLINK *plink = &prec->inp;
    TextFile_t *dpvt = prec->dpvt;
    const char *filename = dpvt->name;

    //
    if (devTextFileAiDebug>0) {
        printf("%s (devTextFileAi): filename: %s\n", prec->name, filename);
    }

    //
    double val = 0;
    long ret = devTextFileRead(filename, &val, (dbCommon *)prec, DBF_DOUBLE, 1, devTextFileAiDebug);

    //
    if (ret < 0) {
        return -1;
    }

    // Apply ASLO & AOFF
    if (prec->aslo != 0.0) {
        val *= prec->aslo;
    }
    val += prec->aoff;

    // Apply smoothing algorithm
    if (prec->smoo != 0.0 && prec->dpvt && finite(prec->val)) {
        prec->val = val * (1.00 - prec->smoo) + (prec->val * prec->smoo);
    } else {
        prec->val = val;
    }

    //
    return 2; // no conversion
}

// Register symbol(s) used by IOC core
epicsExportAddress(int, devTextFileAiDebug);

// end
