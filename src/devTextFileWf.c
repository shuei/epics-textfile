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
        printf("%s (devTextFileWf) filename: %s\n", prec->name, plink->value.instio.string);
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
    char *pstr = plink->value.instio.string;

    if (pstr[0] == '+') {
        // keep file opened and rewind on process
        dpvt->keep = true;
        pstr++;
        //printf("[%s]\n", pstr);
    }

    const size_t fsize = strlen(pstr) + 1;
    //if (fsize > MAX_INSTIO_STRING) {
    //    errlogPrintf("%s (devTextFileWf): INP field is too long\n", prec->name);
    //    return -1;
    //}
    dpvt->name = callocMustSucceed(1, fsize, "calloc for filename failed");
    strcpy(dpvt->name, pstr);

    // Open input file if requested
    if (dpvt->keep) {
        dpvt->fp = fopen(dpvt->name, "r");
        if (dpvt->fp == NULL) {
            errlogPrintf("%s (devTextFileWf): can't open \"%s\"\n", prec->name, dpvt->name);
            prec->pact = 1;
            return -1;
        }
    }

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
    FILE *fp = 0;
    if (dpvt->keep) {
        fp = dpvt->fp;
        if (fseek(fp, 0L, SEEK_SET)!=0) {
            errlogPrintf("%s (devTextFileWf): fseek failed for \"%s\" : %s\n", prec->name, filename, strerror(errno));
            prec->nsev = INVALID_ALARM;
            prec->nsta = READ_ACCESS_ALARM;

            return -1;
        }
        if (fflush(fp)!=0) {
            errlogPrintf("%s (devTextFileWf): fflush failed for \"%s\" : %s\n", prec->name, filename, strerror(errno));
            prec->nsev = INVALID_ALARM;
            prec->nsta = READ_ACCESS_ALARM;

            return -1;
        }
    } else {
        fp = fopen(filename, "r");
        if (fp == NULL) {
            errlogPrintf("%s (devTextFileWf): can't open \"%s\"\n", prec->name, filename);
            prec->nsev = INVALID_ALARM;
            prec->nsta = READ_ACCESS_ALARM;
            return -1;
        }
    }

    int retval = 0;
    char *buf = NULL;
    size_t bufsiz = 0;
    int nline = 0;
    void *bptr = prec->bptr;
    int ival;
    double dval;
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

        //
        if (devTextFileWfDebug>0) {
            printf("%s (devTextFileWf): %d/%d %s", prec->name, n, prec->nelm, pbuf);
        }

        switch (prec->ftvl) {
        case DBF_CHAR:
            if (sscanf(pbuf, "%d", &ival) == 1) {
                int8_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_UCHAR:
            if (sscanf(pbuf, "%u", &ival) == 1) {
                uint8_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_SHORT:
            if (sscanf(pbuf, "%d", &ival) == 1) {
                int16_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_USHORT:
            if (sscanf(pbuf, "%u", &ival) == 1) {
                uint16_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_LONG:
            if (sscanf(pbuf, "%d", &ival) == 1) {
                int32_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_ULONG:
            if (sscanf(pbuf, "%u", &ival) == 1) {
                uint32_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_FLOAT:
            if (sscanf(pbuf, "%lf", &dval) == 1) {
                float *ptr = bptr;
                ptr[n] = dval;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_DOUBLE:
            if (sscanf(pbuf, "%lf", &dval) == 1) {
                double *ptr = bptr;
                ptr[n] = dval;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        default:
            // this may not happen as FTVL is already checked in init_record().
            errlogPrintf("%s (devTextFileWf): unsuppoted FTVL\n", prec->name);
            return(S_db_badField);
        }

        if (n >= prec->nelm) {
            break;
        }
    }

    //
    prec->nord = n; //  number of elements that has been read
    prec->udf = FALSE;

    // check if any data has been read from the input file
    if (n == 0) {
        errlogPrintf("%s (devTextFileWf): no data was read from the file: \"%s\", line %d.\n", prec->name, filename, nline);
        prec->nsev = INVALID_ALARM;
        prec->nsta = READ_ALARM;
        retval = -1;
    }

//    // check if input file reached unexpected end-of-file
//    if (n < prec->nelm) { // This might be too intolerant. Perhaps we'd better to set severity/status in case of n==0 (i.e. nothing has been read).
//        errlogPrintf("%s (devTextFileWf): unexpected end-of-file in \"%s\", line %d.\n", prec->name, filename, nline);
//        prec->nsev = INVALID_ALARM;
//        prec->nsta = READ_ALARM;
//        retval = -1;
//    }

    // cleanup
    if (buf) {
        free(buf);
        buf = NULL;
    }

    if (!dpvt->keep) {
        fclose(fp);
        fp = NULL;
    }

    //
    return retval;
}

// Register symbol(s) used by IOC core
epicsExportAddress(int, devTextFileWfDebug);

// end
