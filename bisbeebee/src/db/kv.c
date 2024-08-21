#include <stdlib.h>
#include <unistd.h>
#include "kv_list.h"

#define MAX_LIST_SIZE 128

int result = 0;

void store_data(const char *key, const void *data, size_t size)
{
  list_store_data(key, data, size);
  result = 1;
  if (list_get_size() >= MAX_LIST_SIZE) {
    // garbage collection
    result = 2;
    list_remove_bad_blocks();
  }
}

ssize_t load_data(const char *key, void *buffer, size_t buffer_size)
{
  return list_load_data(key, buffer, buffer_size);
}

void clear_db()
{
  if (list_get_size() == 0) {
    return;
  }
  clear_db();
}

void initialize()
{
  for (int i = 0; i < 5; ++i) {
    usleep(3000);
  }
}

