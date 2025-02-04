#pragma once
#include "hocstr.h"
#include "nrnmpiuse.h"  // defines USE_NRNFILEWRAP

#include <stdio.h>
#include <stdlib.h>

#if !defined(USE_NRNFILEWRAP)
#define USE_NRNFILEWRAP 0
#endif

#if USE_NRNFILEWRAP
struct NrnFILEWrap {
    FILE* f;
    unsigned char* buf;
    size_t ip, cnt;
};

extern NrnFILEWrap* nrn_fw_wrap(FILE* f);
extern void nrn_fw_delete(NrnFILEWrap* fw);
#define nrn_fw_eq(fw, ff) (fw->f == ff)
extern int nrn_fw_fclose(NrnFILEWrap* fw);
extern NrnFILEWrap* nrn_fw_set_stdin();
extern NrnFILEWrap* nrn_fw_fopen(const char* path, const char* mode);
extern int nrn_fw_fseek(NrnFILEWrap* fw, long offset, int whence);
extern int nrn_fw_getc(NrnFILEWrap* fw);
extern int nrn_fw_ungetc(int c, NrnFILEWrap* fw);
extern int nrn_fw_fscanf(NrnFILEWrap* fw, const char* format, ...);
extern int nrn_fw_readaccess(const char* path);

#else /* not USE_NRNFILEWRAP */
using NrnFILEWrap = FILE;
#define nrn_fw_wrap(f)       f
#define nrn_fw_delete(fw)    /**/
#define nrn_fw_eq(fw, ff)    (fw == ff)
#define nrn_fw_fclose        fclose
#define nrn_fw_set_stdin()   stdin
#define nrn_fw_fopen         fopen
#define nrn_fw_fseek         fseek
#define nrn_fw_getc(fw)      getc(fw)
#define nrn_fw_ungetc(c, fw) ungetc(c, fw)
#define nrn_fw_fscanf        fscanf

#endif
extern char* fgets_unlimited(HocStr* s, NrnFILEWrap* f);
extern NrnFILEWrap* hoc_fin;
