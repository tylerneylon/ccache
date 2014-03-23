// cachetest.c
//

#include "ccache.h"

#include "ctest.h"

#include <stdio.h>
#include <string.h>

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

int test_cache() {
  CCache cache = CCacheNew(str_hash, str_eq, 4);

  // We'll set a->1, b->2, c->3, d->4.
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "a", (void *)1L);
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "b", (void *)2L);
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "c", (void *)3L);
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "d", (void *)4L);
  test_printf("%s:%d\n", __FILE__, __LINE__);

  long value = (long)CCacheGet(cache, "a");
  test_printf("%s:%d\n", __FILE__, __LINE__);
  test_that(value == 1);
  test_printf("%s:%d\n", __FILE__, __LINE__);

  // Push the cache over the limit; the oldest value is now "b".
  CCacheSet(cache, "e", (void *)5L);
  test_printf("%s:%d\n", __FILE__, __LINE__);
  void *value_ptr = CCacheGet(cache, "b");
  test_printf("%s:%d\n", __FILE__, __LINE__);

  test_that(value_ptr == NULL);
  test_printf("%s:%d\n", __FILE__, __LINE__);

  return test_success;
}

static int num_releases = 0;
static void counting_releaser(void *ptr) {
  num_releases++;
}

int test_num_releases() {
  num_releases = 0;
  CCache cache = CCacheNew(str_hash, str_eq, 2);
  cache->map->valueReleaser = counting_releaser;

  // We'll set a->1, b->2, c->3, d->4.
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "a", (void *)1L);
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "b", (void *)2L);
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "c", (void *)3L);
  test_printf("%s:%d\n", __FILE__, __LINE__);
  CCacheSet(cache, "d", (void *)4L);
  test_printf("%s:%d\n", __FILE__, __LINE__);

  test_that(num_releases == 2);

  CCacheDelete(cache);

  test_that(num_releases == 4);

  return test_success;
}


int main(int argc, char **argv) {
  start_all_tests(argv[0]);
  run_tests(test_cache, test_num_releases);
  return end_all_tests();
}