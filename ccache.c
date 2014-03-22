// ccache.c
//

#include "ccache.h"


// Typedefs.

typedef struct {
  void *key;
  void *value;
  CList *age_item;
} KeyValueAge;


// Internal functions.

static void CCacheMakeNewest(CCache cache, KeyValueAge *meta_item) {
  if (meta_item->age_item) {
    CListMoveFirst(meta_item->age_item, &cache->newest);
  } else {
    // It's a new item; insert at oldest iff it's the first-ever item.
    meta_item->age_item = cache->oldest ? &cache->newest->next : &cache->oldest;
    CListInsert(meta_item->age_item, meta_item);
    cache->newest = *(meta_item->age_item);
  }
}

static void CCacheRemoveOldest(CCache cache) {
  KeyValueAge *meta_item = (KeyValueAge *)cache->oldest->element;
  CListRemoveFirst(&cache->oldest);
  CMapUnset(cache->map, meta_item->key);
}

// This will serve as the key-value allocator in our map.
static void *alloc_meta_item(size_t base_size) {
  KeyValueAge *meta_item = malloc(sizeof(KeyValueAge));
  meta_item->age_item = NULL;
  return meta_item;
}


// Public functions.

CCache CCacheNew(Hash hash, Eq eq, unsigned int max_count) {
  CCache cache = malloc(sizeof(CCacheStruct));
  cache->map = CMapNew(hash, eq);
  cache->map->pairAlloc = alloc_meta_item;
  cache->newest = cache->oldest = NULL;
  cache->max_count = max_count;
  return cache;
}

void CCacheDelete(CCache cache) {
  CListDelete(&cache->oldest);
  CMapDelete(cache->map);
  free(cache);
}

void *CCacheGet(CCache cache, void *key) {
  KeyValueAge *meta_item = (KeyValueAge *)CMapFind(cache->map, key);
  if (meta_item == NULL) return NULL;
  CCacheMakeNewest(cache, meta_item);
  return meta_item->value;
}

void CCacheSet(CCache cache, void *key, void *value) {
  KeyValueAge *meta_item = (KeyValueAge *)CMapSet(cache->map, key, value);
  CCacheMakeNewest(cache, meta_item);
  if (cache->map->count > cache->max_count) CCacheRemoveOldest(cache);
}

