// ccache.h
//
// https://github.com/tylerneylon/ccache
//
// A least-recently-used (LRU) cache.
// Built on top of cstructs.
//

#include "cstructs/cstructs.h"

typedef struct {
  Map  map;
  List newest;
  List oldest;
  int  max_count;
} CCacheStruct;

typedef CCacheStruct *CCache;

// Access the Map `map` directly to set up a key- or value-releaser.
CCache CCacheNew(map__Hash hash, map__Eq eq, unsigned int max_count);
void   CCacheDelete(CCache cache);

void *CCacheGet(CCache cache, void *key);
void  CCacheSet(CCache cache, void *key, void *value);
