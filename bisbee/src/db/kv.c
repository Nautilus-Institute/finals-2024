#include <stdlib.h>
#include "kv_list.h"
#include "kv_hashmap.h"

#define MAX_LIST_SIZE 128

struct global_data_t {
  int mode;  // 0: list, 1: hashmap

  long long hash_map_created;
  HashMap hash_map;

  // function pointer for getting size
  void (*init)(void);
  void (*list_to_hashmap)(void);
  int (*get_storage_size)(void);
  void (*clear_db)(void);
};

struct global_data_t g;

int hh_get_size_local()
{
  return hh_get_size(&g.hash_map);
}

void hh_clear_db_local()
{
  if (g.hash_map_created) {
    free_hash_map(&g.hash_map);
  }
}

void list_to_hashmap()
{
  hh_create(&g.hash_map);
  for (int i = 0; i < g.get_storage_size(); i++) {
    const char *key = list_get_key(i);
    const void *data = list_get_data(i);
    size_t size = list_get_data_size(i);
    hh_store_data(&g.hash_map, key, data, size);
  }
  g.clear_db();
  g.clear_db = hh_clear_db_local;
  g.get_storage_size = hh_get_size_local;
}

void store_data(const char *key, const void *data, size_t size)
{
  int result = 0;
  if (g.mode == 0) {
    list_store_data(key, data, size);
    result = 1;
    if (g.get_storage_size() >= MAX_LIST_SIZE) {
      // switch to hash map
      g.mode = 1;
      list_to_hashmap();
    }
    result = 2;
  } else {
    if (!g.hash_map_created) {
      hh_create(&g.hash_map);
    }
    hh_store_data(&g.hash_map, key, data, size);
    result = 2;
  }
  // return result;
}

ssize_t load_data(const char *key, void *buffer, size_t buffer_size)
{
  if (g.mode == 0) {
    return list_load_data(key, buffer, buffer_size);
  } else {
    if (!g.hash_map_created) {
      hh_create(&g.hash_map);
    }
    return hh_load_data(&g.hash_map, key, buffer, buffer_size);
  }
}

void clear_db()
{
#ifdef DEBUG
  printf("g.init = %p\n", g.init);
  printf("g.list_to_hashmap = %p\n", g.list_to_hashmap);
  printf("g.get_storage_size = %p\n", g.get_storage_size);
  printf("g.clear_db = %p\n", g.clear_db);
  printf("\n\n\n");
#endif
  if (g.get_storage_size() == 0) {
    return;
  }
  g.clear_db();
}

void initialize()
{
  g.mode = 0;
  g.hash_map_created = 0;
  g.init = NULL;
  g.list_to_hashmap = list_to_hashmap;
  g.get_storage_size = list_get_size;
  g.clear_db = list_clear_db;
#ifdef DEBUG
  printf("&g.init = %p\n", &g.init);
#endif
}

