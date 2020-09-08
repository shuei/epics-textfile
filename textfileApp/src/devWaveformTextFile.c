/* devWaveformTextFile.c */
/****************************************************************************
 *                         COPYRIGHT NOTIFICATION
 *
 * devWaveformTextFile: A part of EPICS device support for FA-M3 PLC made by
 * Yokogawa
 * Copyright (C) 2004  Jun-ichi Odagiri
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ****************************************************************************/
/* Author: Jun-ichi Odagiri */
/* Modification Log:
 * -----------------
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "dbCommon.h"
#include "waveformRecord.h"
#include "dbAccess.h"
#include "devSup.h"
#include "recGbl.h"
#include "link.h"
#include "errlog.h"

#ifndef EPICS_REVISION
#include <epicsVersion.h>
#endif
#if EPICS_REVISION == 14 && EPICS_MODIFICATION >= 2
#include <epicsExport.h>
#endif

#define ERROR  (-1)
#define MAX_INSTIO_STRING  256

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
} devWfTextFile = {
  6,
  NULL,
  init,
  init_record,
  NULL,
  read_wf,
  NULL
};

#if EPICS_REVISION == 14 && EPICS_MODIFICATION >= 2
epicsExportAddress(dset, devWfTextFile);
#endif

static int sizeofTypes[] = {0,1,1,2,2,4,4,4,8,2};

static long init(void)
{
  return 0;
}

static long init_record(struct waveformRecord *pwf)
{
  struct link *plink = &pwf->inp;

  if (plink->type != INST_IO) {
//      errlogPrintf("devNetDev: address type must be \"INST_IO\"\n");
      errlogPrintf("devWaveformtextFile: address type must be \"INST_IO\"\n");
      return ERROR;
  }

  unsigned long fsize = strlen(plink->value.instio.string) + 1;
  if (fsize > MAX_INSTIO_STRING) {
    errlogPrintf("devWaveformTextFile: instio.string is too long\n");
    return ERROR;
  }

  return 0; // 2013 Jun 05 - shuei
}


static long read_wf(struct waveformRecord *pwf)
{
  struct link *plink = &pwf->inp;
  uint8_t *bptr = (uint8_t *) pwf->bptr;
  int size = sizeofTypes[pwf->ftvl];

  /*
  printf("devWaveformTextFile:\"%s\", nelm:%d, fsize:%d\n",pwf->name, pwf->nelm, size);
  */

  unsigned long fsize = strlen(plink->value.instio.string) + 1;
  if (fsize > MAX_INSTIO_STRING) {
    errlogPrintf("devWaveformTextFile: instio.string is too long\n");
    return ERROR;
  }

  char *filename = calloc(1, fsize);
  if (!filename) {
    errlogPrintf("devWaveformTextFile: can't calloc for filename\n");
    return ERROR;
  }

  strncpy(filename, plink->value.instio.string, fsize);
  filename[fsize - 1] = '\0';

  /*
  printf("devWaveformTextFile: filename[%d]: %s\n",(int)fsize,filename);
  */

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    errlogPrintf("devWaveformTextFile: can't open \"%s\"\n", filename);
    return ERROR;
  }

  char *line = NULL;
  size_t len = 0;
  int ival;
  double dval;
  int8_t *p8;
  uint8_t *pu8;
  int16_t *p16;
  uint16_t *pu16;
  int32_t *p32;
  uint32_t *pu32;
  float *pflt;
  double *pdbl;
  unsigned long n = 0;
  ssize_t nchars;
  while ((nchars = getline(&line, &len, fp)) != -1) {
    /*
      printf("devWaveformTextFile: line(%d):%s", n, line);
    */
    switch (pwf->ftvl) {
    case DBF_CHAR:
       sscanf(line, "%d", &ival);
       p8 = &bptr[size * n];
       *p8 = (int8_t) ival;
       break;
    case DBF_UCHAR:
       sscanf(line, "%u", &ival);
       pu8 = &bptr[size * n];
       *pu8 = (uint8_t) ival;
       break;
    case DBF_SHORT:
       sscanf(line, "%d", &ival);
       p16 = &bptr[size * n];
       *p16 = (int16_t) ival;
       break;
    case DBF_USHORT:
       sscanf(line, "%u", &ival);
       pu16 = &bptr[size * n];
       *pu16 = (uint16_t) ival;
       break;
    case DBF_LONG:
       sscanf(line, "%d", &ival);
       p32 = &bptr[size * n];
       *p32 = (int32_t) ival;
       break;
    case DBF_ULONG:
       sscanf(line, "%u", &ival);
       pu32 = &bptr[size * n];
       *pu32 = (uint32_t) ival;
       break;
    case DBF_FLOAT:
       sscanf(line, "%lf", &dval);
       pflt = &bptr[size * n];
       *pflt = (float)  dval;
       break;
    case DBF_DOUBLE:
       sscanf(line, "%lf", &dval);
       pdbl = &bptr[size * n];
       *pdbl = (double) dval;
       break;
    default:
       errlogPrintf("devWaveformTextFile: unsuppoted FTVL\n");
       return(S_db_badField);
    }

    if (n++ >= pwf->nelm) break;

  }

  if (line) free(line);

  fclose(fp);

  if (filename) free(filename);

  pwf->nord = n;

  return 0;
}
