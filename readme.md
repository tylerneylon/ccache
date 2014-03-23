# ccache
*An efficient LRU cache.*

This library provides a fast cache that
purges oldest items when its customized
`max_count` is exceeded.
It's built on top of
[cstructs](https://github.com/tylerneylon/cstructs),
which uses `void *` for keys and values, and supports
use of this space as either direct values (cast to a pointer),
or as pointers to custom objects.

Below is a usage example using strings as
keys and integers as values.

```
// Set up hash and equality-check functions for strings.

int str_hash(void *str_void_ptr) {
  char *str = (char *)str_void_ptr;
  int h = *str;
  while (*str) {
    h *= 84207;
    h += *str++;
  }
  return h;
}

int str_eq(void *str_void_ptr1, void *str_void_ptr2) {
  return !strcmp(str_void_ptr1, str_void_ptr2);
}

static CCache cache = NULL;

long my_func(char *key) {
  // Make sure we have a cache that holds up to 64 values.
  if (cache == NULL) cache = CCacheNew(str_hash, str_eq, 64);

  void *cache_value = CCacheGet(cache, key);
  if (cache_value) return (long)cache_value;

  // No cached value; compute it.
  long value = my_func_(key);

  CCacheSet(cache, key, (void *)value);
  return value;
}
```

Note that storing direct values cast to `void *` makes it
impossible to distinguish between a value of zero and a missing
cache value. If you need to distinguish between the two, you
can dynamically allocate the values you need and store pointers
to them. You can set a value releaser function like so:

    cache->map->valueReleaser = free;

and `cache` will call this releaser for every value that is
purged when the cache is full; there is an analogous
`keyReleaser` function as well. The example above assumes
the keys (strings) will live on as long as needed in the cache,
which may be a bad assumption. Using a `keyReleaser` and giving
the cache ownership of the keys (such as using `strdup`) is one
way to improve on this.
