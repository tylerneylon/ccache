// ccache.c
//
// https://github.com/tylerneylon/ccache
//

#include "ccache.h"


#define true  1
#define false 0

// The well-formed check is incredibly slow, so it is not even
// on during DEBUG mode; rather it is reserved for builds
// specifically designed to check a cache's integrity.
#ifdef CCACHE_TESTING
#define CCacheIsWellFormed(x) CCacheIsWellFormed_(x, __LINE__)
#else
#define CCacheIsWellFormed(x)
#endif
int ccache_error_line = 0;


// Typedefs.

typedef struct {
  void *key;
  void *value;
  List *age_item;
} KeyValueAge;


// Internal functions.

int fail(int line) {
  ccache_error_line = line;
  return false;
}

int CCacheIsWellFormed_(CCache cache, int line) {
    
  // Check that the oldest->newest list is well-formed.
  List *prev = &cache->oldest;
  list__for(KeyValueAge *, meta_item, cache->oldest) {
    if (meta_item->age_item != prev) return fail(line);
    prev = &(*prev)->next;
  }
  if (cache->newest) {
    if (prev != &cache->newest->next) return fail(line);
  } else {
    // Cache should be empty.
    if (cache->oldest || cache->map->count) return fail(line);
  }
  
  // Check that the map.has well-formed KeyValueAge entries.
  map__for(pair, cache->map) {
    KeyValueAge *meta_item = (KeyValueAge *)pair;
    if ((*meta_item->age_item)->item != meta_item) return fail(line);
  }
  
  return true;
}

static void CCacheMakeNewest(CCache cache, KeyValueAge *meta_item) {
  if (meta_item->age_item) {
    List next = (*meta_item->age_item)->next;
    if (next == NULL) return;  // Already newest.
    ((KeyValueAge *)next->item)->age_item = meta_item->age_item;
    list__move_first(meta_item->age_item, &cache->newest->next);
    meta_item->age_item = &cache->newest->next;
  } else {
    // It's a new item; insert at oldest iff it's the first-ever item.
    meta_item->age_item = cache->oldest ? &cache->newest->next : &cache->oldest;
    list__insert(meta_item->age_item, meta_item);
  }
  cache->newest = *(meta_item->age_item);
  CCacheIsWellFormed(cache);
}

static void CCacheRemoveOldest(CCache cache) {
  KeyValueAge *meta_item = (KeyValueAge *)cache->oldest->item;
  list__remove_first(&cache->oldest);
  // cache->oldest exists since max_count >= 1, and we wait till the
  // buffer is overfill by 1 (internally) before removing the oldest.
  ((KeyValueAge *)cache->oldest->item)->age_item = &cache->oldest;
  map__unset(cache->map, meta_item->key);
  CCacheIsWellFormed(cache);
}

// This will serve as the key-value allocator in our map.
static void *alloc_meta_item(size_t base_size) {
  KeyValueAge *meta_item = malloc(sizeof(KeyValueAge));
  meta_item->age_item = NULL;
  return meta_item;
}


// Public functions.

CCache CCacheNew(map__Hash hash, map__Eq eq, unsigned int max_count) {
  CCache cache = malloc(sizeof(CCacheStruct));
  cache->map = map__new(hash, eq);
  cache->map->pair_alloc = alloc_meta_item;
  cache->newest = cache->oldest = NULL;
  cache->max_count = max_count;
  return cache;
}

void CCacheDelete(CCache cache) {
  list__delete(&cache->oldest);
  map__delete(cache->map);
  free(cache);
}

void *CCacheGet(CCache cache, void *key) {
  KeyValueAge *meta_item = (KeyValueAge *)map__get(cache->map, key);
  if (meta_item == NULL) return NULL;
  CCacheMakeNewest(cache, meta_item);
  CCacheIsWellFormed(cache);
  return meta_item->value;
}

void CCacheSet(CCache cache, void *key, void *value) {
  KeyValueAge *meta_item = (KeyValueAge *)map__set(cache->map, key, value);
  CCacheMakeNewest(cache, meta_item);
  if (cache->map->count > cache->max_count) CCacheRemoveOldest(cache);
  CCacheIsWellFormed(cache);
}

