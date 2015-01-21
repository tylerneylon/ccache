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

## Example

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
to them. You can define a releaser with the following type
layout:

    void my_releaser(void *item, void *context) {
      /* release `item` here; `context` will always be NULL */
    }

You can set a value releaser function like so:

    cache->map->value_releaser = my_releaser;

and `cache` will call this releaser for every value that is
purged when the cache is full; there is an analogous
`key_releaser` function as well. The example above assumes
the keys (strings) will live on as long as needed in the cache,
which may be a bad assumption. Using a `key_releaser` and giving
the cache ownership of the keys (such as using `strdup`) is one
way to improve on this.

## Internal layout

These details may be useful for contributors; they're not
necessary to know about to use the library.

The primary structure is a `CMap` that has its key-value
pairs augmented with a `CList *` called `age_item`.
Conceptually, an `age_item` captures how long ago the
key-value pair was last referred to; in the code, it
provides a fast way for us to move an item to the newest
side of the age list.

We keep two pointers into the age list: `newest` and
`oldest`. We use `oldest` to purge old items when incoming
items would push us over `max_count`. We use `newest` to
know where to insert new incoming items. The entire
thing is a single `CList` ordered by age.

Here's a sample data diagram with two key-value pairs:

```
                           +––––––––+             
                           | newest |             
                           +––––––––+             
                              |                   
             CListStruct      v                   
+––––––––+   +–––––––––+   +–––––––––+            
| oldest |-> | next(1) |-> | next(2) |-|          
+––––––––+   +–––––––––+   +–––––––––+            
    ^        | element |   | element |            
    |        +–––––––––+   +–––––––––+            
    |          |             |                    
    |          v             v                    
    |        +––––––––––+  +––––––––––+           
    |        | key      |  | key      |           
    |        +––––––––––+  +––––––––––+           
    |        | value    |  | value    |           
    |        +––––––––––+  +––––––––––+           
    +––––––––+ age_item |  | age_item |->[next(1)]
             +––––––––––+  +––––––––––+           
             KeyValueAge                          
```

This chart was built with the [asciiflow](http://asciiflow.com/) tool.
