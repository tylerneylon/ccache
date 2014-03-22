#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#define malloc_size malloc_usable_size 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memprofile.h"

#define tableSize 500

static int byteDelta[tableSize];
static char rowFile[tableSize][512];
static int rowLine[tableSize];
static int isZeroed = 0;

#undef malloc
#undef realloc
#undef free

int rowNum(char *file, int line) {
  char *c = file;
  unsigned int val = 0;
  while (*c++) {
    val *= 123;
    val += *c;
  }
  val *= 93;
  val += line;
  return val % tableSize;
}

void *memop(char *file, int line, void *ptr, int numBytes, int isRealloc) {
  if (!isZeroed) {
    for (int i = 0; i < tableSize; ++i) byteDelta[i] = 0;
    isZeroed = 1;
  }
  int row = rowNum(file, line);
  strncpy(rowFile[row], file, 511);
  rowLine[row] = line;
  if (isRealloc) {
    int prevSize = (int)malloc_size(ptr);
    void *vp = realloc(ptr, numBytes);
    byteDelta[row] += (malloc_size(vp) - prevSize);
    return vp;
  }
  if (numBytes >= 0) {
    void *vp = malloc(numBytes);
    byteDelta[row] += malloc_size(vp);
    return vp;
  } else {
    byteDelta[row] -= malloc_size(ptr);
    free(ptr);
    return NULL;
  }
}

void printmeminfo() {
  int totalDelta = 0;

  int fileNet[128];
  char files[128][512];
  int numFiles = 0;

  for (int i = 0; i < tableSize; ++i) {
    if (byteDelta[i] != 0) {
      totalDelta += byteDelta[i];
      printf("%26s:%5d: %10d\n", rowFile[i], rowLine[i], byteDelta[i]);

      int fileIndex = -1;
      for (int j = 0; j < numFiles; ++j) {
        if (strcmp(files[j], rowFile[i]) == 0) {
          fileIndex = j;
          break;
        }
      }
      if (fileIndex == -1) {
        fileIndex = numFiles++;
        strcpy(files[fileIndex], rowFile[i]);
        fileNet[fileIndex] = 0;
      }
      fileNet[fileIndex] += byteDelta[i];
    }
  }
  printf("%32s: %10d\n", "total", totalDelta);
  printf("\nPer file net:\n");
  for (int i = 0; i < numFiles; ++i) {
    printf("%32s: %10d\n", files[i], fileNet[i]);
  }

#ifndef __APPLE__
  printf("malloc_stats:\n");
  malloc_stats();
#endif

}

