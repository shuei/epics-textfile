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
#include "cantProceed.h"
#include "dbAccess.h"
#include "dbCommon.h"
#include "devSup.h"
#include "alarm.h"
#include "errlog.h"
#include "recGbl.h"
#include "link.h"

//
#include "devTextFile.h"

/////////////////////////////////////////////////////////////////
//
// Read data from file and fill to record buffer
//
long devTextFileRead(const char *filename, void *bptr, dbCommon *prec, int ftvl, int nelm, int debug)
{
    //DBLINK *plink = &prec->inp;
    TextFile_t *dpvt = prec->dpvt;
    const char *ftvlstr = (pamapdbfType[ftvl].strvalue) + 4;

    //
    if (debug > 0) {
        printf("%s (%s): filename: %s ftvl=%s nelm=%d\n", prec->name, __func__, filename, ftvlstr, nelm);
    }

    //
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
        errlogPrintf("%s (%s): can't open \"%s\" for reading: %s\n", prec->name, __func__, filename, errmsg);
        prec->nsev = INVALID_ALARM;
        prec->nsta = READ_ACCESS_ALARM;
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
        while (isspace(*pbuf)) {
            pbuf ++;
        }

        // skip empty lines.
        if (strlen(pbuf) == 0) {
            continue;
        }

        // skip comments.
        if (pbuf[0] == '#' || pbuf[0] == ';' || pbuf[0] == '!') {
            continue;
        }

        //
        if (debug > 0) {
            printf("%s (%s): %d/%d %s", prec->name, __func__, n+1, nelm, pbuf);
        }

        //
        char *endptr = 0;
        errno = 0;

        if (0) {
            //
        } else if (ftvl == DBF_STRING) {
            char *val = bptr;
            strncpy(val, pbuf, MAX_STRING_SIZE);
            val[MAX_STRING_SIZE-1] = 0;

            char *p = strchr(val, '\n');
            if (p) {
                *p = 0;
            }
            n++;
        } else if (ftvl == DBF_CHAR) {
            int val = strtol(pbuf, &endptr, 0);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr == pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                int8_t *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else if (ftvl == DBF_UCHAR) {
            int val = strtol(pbuf, &endptr, 0);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr==pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                uint8_t *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else if (ftvl == DBF_SHORT) {
            int val = strtol(pbuf, &endptr, 0);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr==pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                int16_t *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else if (ftvl == DBF_USHORT) {
            int val = strtol(pbuf, &endptr, 0);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr==pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                uint16_t *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else if (ftvl == DBF_LONG) {
            int val = strtol(pbuf, &endptr, 0);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr==pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                int32_t *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else if (ftvl == DBF_ULONG) {
            int val = strtol(pbuf, &endptr, 0);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr==pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                uint32_t *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else if (ftvl == DBF_FLOAT) {
            double val = strtod(pbuf, &endptr);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr==pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                float *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else if (ftvl == DBF_DOUBLE) {
            double val = strtod(pbuf, &endptr);
            if (errno != 0) {
                char *errmsg = strerror_r(errno, dpvt->errmsg, ERRBUF); // GNU-specific version is assumed
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: %s\n", prec->name, __func__, filename, nline, errmsg);
            } else if (endptr==pbuf) {
                errlogPrintf("%s (%s): parse error in \"%s\", line %d: No digits were found\n", prec->name, __func__, filename, nline);
            } else {
                // Read succeeded
                double *ptr = bptr;
                ptr[n] = val;
                n++;
            }
        } else {
            //
            errlogPrintf("%s (%s): unsuppoted FTVL\n", prec->name, __func__);
            break;
        }

        if (n >= nelm) {
            break;
        }
    }

    //
    //prec->nord = n; //  number of elements that has been read
    prec->udf = FALSE;

    // check if any data has been read from the input file
    if (n == 0) {
        errlogPrintf("%s (%s): No data was read from the file: \"%s\"\n", prec->name, __func__, filename);
        prec->nsev = INVALID_ALARM;
        prec->nsta = READ_ALARM;
    }

//    // check if input file reached unexpected end-of-file
//    if (n < prec->nelm) { // This might be too intolerant. Perhaps we'd better to set severity/status in case of n==0 (i.e. nothing has been read).
//        errlogPrintf("%s (%s): unexpected end-of-file in \"%s\", line %d.\n", prec->name, __func__, filename, nline);
//        prec->nsev = INVALID_ALARM;
//        prec->nsta = READ_ALARM;
//        retval = -1;
//    }

    // cleanup
    if (buf) {
        free(buf);
        buf = NULL;
    }

    fclose(fp);
    fp = NULL;

    //
    if (debug > 0) {
        printf("%s (%s): ret = %d \n", prec->name, __func__, n);
    }

    //
    return n;
}

// end
