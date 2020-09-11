/* devTextFileWf.c */
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
  Current Author: Shuei Yamada (shuei@post.kek.jp)
  Original Author: Jun-ichi Odagiri
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//
#include <waveformRecord.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <errlog.h>
//#include <epicsVersion.h>
#include <epicsExport.h>

#define MAX_INSTIO_STRING  256
#undef DEBUG

/***************************************************************
 * Waveform (command/response IO)
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
//static int sizeofTypes[] = {0,1,1,2,2,4,4,4,8,2};

static long init(void)
{
  return 0;
}

//
static long init_record(struct waveformRecord *prec)
{
    DBLINK *plink = &prec->inp;

#ifdef DEBUG
    printf("%s (devTextFileWf) filename: %s\n", prec->name, plink->value.instio.string);
#endif

    if (plink->type != INST_IO) {
        errlogPrintf("%s (devTextFileWf): address type must be \"INST_IO\"\n", prec->name);
        return -1;
    }

    unsigned long fsize = strlen(plink->value.instio.string) + 1;
    if (fsize > MAX_INSTIO_STRING) {
        errlogPrintf("%s (devTextFileWf): instio.string is too long\n", prec->name);
        return -1;
    }

    return 0;
}

//
static long read_wf(struct waveformRecord *prec)
{
    DBLINK *plink = &prec->inp;
    //int size = sizeofTypes[prec->ftvl];

#ifdef DEBUG
    printf("%s (devTextFileWf): nelm:%d, fsize:%d\n",prec->name, prec->nelm, size);
#endif

    unsigned long fsize = strlen(plink->value.instio.string) + 1;
    if (fsize > MAX_INSTIO_STRING) {
        errlogPrintf("%s (devTextFileWf): instio.string is too long\n", prec->name);
        return -1;
    }

    char *filename = calloc(1, fsize);
    if (!filename) {
        errlogPrintf("%s (devTextFileWf): can't calloc for filename %s\n", prec->name, plink->value.instio.string);
        return -1;
    }

    strncpy(filename, plink->value.instio.string, fsize);
    filename[fsize - 1] = '\0';

#ifdef DEBUG
    printf("%s (devTextFileWf): filename: %s\n", prec->name, filename);
#endif

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        errlogPrintf("%s (devTextFileWf): can't open \"%s\"\n", prec->name, filename);
        return -1;
    }

    char *buf = NULL;
    size_t bufsiz = 0;
    int nline = 0;
    void *bptr = prec->bptr;
    int ival;
    double dval;
    unsigned long n = 0;
    ssize_t nchars;

    while ((nchars = getline(&buf, &bufsiz, fp)) != -1) {
        nline ++;

        switch (prec->ftvl) {
        case DBF_CHAR:
            if (sscanf(buf, "%d", &ival) == 1) {
                int8_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_UCHAR:
            if (sscanf(buf, "%u", &ival) == 1) {
                uint8_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_SHORT:
            if (sscanf(buf, "%d", &ival) == 1) {
                int16_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_USHORT:
            if (sscanf(buf, "%u", &ival) == 1) {
                uint16_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_LONG:
            if (sscanf(buf, "%d", &ival) == 1) {
                int32_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_ULONG:
            if (sscanf(buf, "%u", &ival) == 1) {
                uint32_t *ptr = bptr;
                ptr[n] = ival;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_FLOAT:
            if (sscanf(buf, "%lf", &dval) == 1) {
                float *ptr = bptr;
                ptr[n] = dval;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        case DBF_DOUBLE:
            if (sscanf(buf, "%lf", &dval) == 1) {
                double *ptr = bptr;
                ptr[n] = dval;
                n++;
            } else {
                errlogPrintf("%s (devTextFileWf): parse error in \"%s\", line %d.\n", prec->name, filename, nline);
            }
            break;
        default:
            errlogPrintf("%s (devTextFileWf): unsuppoted FTVL\n", prec->name);
            return(S_db_badField);
        }

        if (n >= prec->nelm) {
            break;
        }
    }

    // cleanup
    if (buf) {
        free(buf);
    }

    fclose(fp);

    if (filename) {
        free(filename);
    }

    prec->nord = n;

    return 0;
}
