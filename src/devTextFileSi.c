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
#include "stringinRecord.h"
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
static int devTextFileSiDebug = 0;

/***************************************************************
 * stringin (command/response IO)
 ***************************************************************/
static long init(void);
static long init_record(struct stringinRecord *);
static long read_si(struct stringinRecord *);

struct {
    long        number;
    DEVSUPFUN   report;
    DEVSUPFUN   init;
    DEVSUPFUN   init_record;
    DEVSUPFUN   get_ioint_info;
    DEVSUPFUN   read_si;
    DEVSUPFUN   special_linconv;
} devTextFileSi = {
    6,
    NULL,
    init,
    init_record,
    NULL,
    read_si,
    NULL
};

epicsExportAddress(dset, devTextFileSi);

//
static long init(void)
{
    return 0;
}

//
static long init_record(struct stringinRecord *prec)
{
    DBLINK *plink = &prec->inp;

    //
    if (devTextFileSiDebug>0) {
        printf("%s (devTextFileSi) filename: %s\n", prec->name, plink->value.instio.string);
    }

    // Link type must be INST_IO
    if (plink->type != INST_IO) {
        errlogPrintf("%s (devTextFileSi): address type must be \"INST_IO\"\n", prec->name);
        prec->pact = 1;
        return -1;
    }

    // Allocate private data storage area
    TextFile_t *dpvt = callocMustSucceed(1, sizeof(TextFile_t), "calloc for private_t failed ");
    prec->dpvt = dpvt;

    // Extract input filename
    const char *pstr = plink->value.instio.string;
    const size_t fsize = strlen(pstr) + 1;
    //if (fsize > MAX_INSTIO_STRING) {
    //    errlogPrintf("%s (devTextFileSi): INP field is too long\n", prec->name);
    //    return -1;
    //}
    dpvt->name = callocMustSucceed(1, fsize, "calloc for filename failed");
    strcpy(dpvt->name, pstr);

    //
    return 0;
}

//
static long read_si(struct stringinRecord *prec)
{
    //DBLINK *plink = &prec->inp;
    TextFile_t *dpvt = prec->dpvt;
    const char *filename = dpvt->name;

    //
    if (devTextFileSiDebug>0) {
        printf("%s (devTextFileSi): filename: %s\n", prec->name, filename);
    }

    //
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        errlogPrintf("%s (devTextFileSi): can't open \"%s\"\n", prec->name, filename);
        prec->nsev = INVALID_ALARM;
        prec->nsta = READ_ACCESS_ALARM;
        return -1;
    }

    int retval = 0;
    char *buf = NULL;
    size_t bufsiz = 0;
    int nline = 0;
    uint32_t n = 0;
    ssize_t nchars;

    while ((nchars = getline(&buf, &bufsiz, fp)) != -1) {
        nline ++;
        char *pbuf = buf;

        // skip until non white-space character.
        while (isspace(*pbuf)) {
            pbuf ++;
        }

        // skip empty lines.
        if (strlen(pbuf)==0) {
            continue;
        }

        // skip comments.
        if (pbuf[0]=='#' || pbuf[0]==';' || pbuf[0]=='!') {
            continue;
        }

        //
        if (devTextFileSiDebug>0) {
            printf("%s (devTextFileSi): %d %s", prec->name, n, pbuf);
        }

        //
        strncpy(prec->val, pbuf, MAX_STRING_SIZE);
        prec->val[MAX_STRING_SIZE-1] = 0;

        char *p = strchr(prec->val, '\n');
        if (p) {
            *p = 0;
        }

        n++;
        break;
    }

    //
    prec->udf = FALSE;

    // check if any data has been read from the input file
    if (n == 0) {
        errlogPrintf("%s (devTextFileSi): No data was read from the file: \"%s\"\n", prec->name, filename);
        prec->nsev = INVALID_ALARM;
        prec->nsta = READ_ALARM;
        retval = -1;
    }

    // cleanup
    if (buf) {
        free(buf);
        buf = NULL;
    }

    fclose(fp);
    fp = NULL;

    //
    return retval;
}

// Register symbol(s) used by IOC core
epicsExportAddress(int, devTextFileSiDebug);

// end
