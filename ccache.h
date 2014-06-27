// ccache.h
//
// Home repo: https://github.com/tylerneylon/ccache
//
// A least-recently-used (LRU) cache.
// Built on top of cstructs.
//

#include "cstructs/cstructs.h"

typedef struct {
  CMap map;
  CList newest;
  CList oldest;
  int max_count;
} CCacheStruct;

typedef CCacheStruct *CCache;

// Access the CMap `map` directly to set up a key- or value-releaser.
CCache CCacheNew(Hash hash, Eq eq, unsigned int max_count);
void CCacheDelete(CCache cache);

void *CCacheGet(CCache cache, void *key);
void CCacheSet(CCache cache, void *key, void *value);
