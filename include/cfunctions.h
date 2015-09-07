#ifndef __C_FUNCTIONS
#define __C_FUNCTIONS
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void SetLogFile(const char* filename);
char* GetLogFile();
void FreeLogFile();

void WriteLog(const char* str, ...);
char* getTime();

#ifdef __cplusplus
}
#endif

#endif
