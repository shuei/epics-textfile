/* devTextFileAi.c */
/****************************************************************************
 *                         COPYRIGHT NOTIFICATION
 *
 * Copyright (c) All rights reserved
 *
 * EPICS BASE Versions 3.13.7
 * and higher are distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 ****************************************************************************/
/*
  Author: Shuei YAMADA (shuei@post.kek.jp)
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

//
#include "aiRecord.h"
#include "dbAccess.h"
#include "devSup.h"
#include "alarm.h"
#include "errlog.h"
#include "recGbl.h"
#include "link.h"
//#include <epicsVersion.h>
#include <epicsExport.h>

//
#include "devTextFile.h"
#undef DEBUG

/***************************************************************
 * Ai (command/response IO)
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

#ifdef DEBUG
    printf("%s (devTextFileWf) filename: %s\n", prec->name, plink->value.instio.string);
#endif

    if (plink->type != INST_IO) {
        errlogPrintf("%s (devTextFileAi): address type must be \"INST_IO\"\n", prec->name);
        return -1;
    }

    unsigned long fsize = strlen(plink->value.instio.string) + 1;
    if (fsize > MAX_INSTIO_STRING) {
        errlogPrintf("%s (devTextFileAi): INP field is too long\n", prec->name);
        return -1;
    }

    return 0;
}

//
static long read_ai(struct aiRecord *prec)
{
    DBLINK *plink = &prec->inp;

    unsigned long fsize = strlen(plink->value.instio.string) + 1;
    if (fsize > MAX_INSTIO_STRING) {
        errlogPrintf("%s (devTextFileAi): INP field is too long\n", prec->name);
        return -1;
    }

    char *filename = calloc(1, fsize);
    if (!filename) {
        errlogPrintf("%s (devTextFileAi): can't calloc for filename \"%s\"\n", prec->name, plink->value.instio.string);
        return -1;
    }

    strncpy(filename, plink->value.instio.string, fsize);
    filename[fsize - 1] = '\0';

#ifdef DEBUG
    printf("%s (devTextFileAi): filename: %s\n", prec->name, filename);
#endif

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        errlogPrintf("%s (devTextFileAi): can't open \"%s\"\n", prec->name, filename);
        return -1;
    }

    char *buf = NULL;
    size_t bufsiz = 0;
    int nline = 0;
    uint32_t n = 0;
    ssize_t nchars;

    while ((nchars = getline(&buf, &bufsiz, fp)) != -1) {
        nline ++;
        char *pbuf = buf;

        // skip until non white-space character.
        while(isspace(*pbuf)) {
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

#ifdef DEBUG
        printf("%s (devTextFileAi): %d %s", prec->name, n, pbuf);
#endif

        //
        double val = 0;
        if (sscanf(pbuf, "%lf", &val) == 1) {
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

            prec->udf = FALSE;
            prec->dpvt = &devTextFileAi; // Any non-zero value

            n++;
            break;
        } else {
            errlogPrintf("%s (devTextFileAi): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
        }
    }

    // check if input file was too short
    if (n == 0) {
        errlogPrintf("%s devTextFileAi): unexpected end-of-file in \"%s\", line %d.\n", prec->name, filename, nline);
        prec->nsev = INVALID_ALARM;
        prec->nsta = READ_ALARM;
        return 0;
    }

    // cleanup
    if (buf) {
        free(buf);
        buf = NULL;
    }

    if (fp) {
        fclose(fp);
        fp = NULL;
    }

    if (filename) {
        free(filename);
        filename = NULL;
    }

    //
    return 2; // no conversion
}
