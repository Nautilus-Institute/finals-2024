#ifndef KV_HASHMAP_H
#define KV_HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 1024

typedef struct Entry {
  size_t size;
  char *key;
  union {
    void *data;
    char short_data[8];
  } u;
  struct Entry *next;
} Entry;

typedef struct HashMap {
  int count;
  Entry table[TABLE_SIZE];
} HashMap;

void hh_create(HashMap *hash_map);

void hh_store_data(HashMap *hash_map, const char *key, const void *data, size_t size);

int hh_load_data(HashMap *hash_map, const char *key, void *buffer, size_t buffer_size);

void hh_clear_db(HashMap *hash_map);

void free_hash_map(HashMap *hash_map);

int hh_get_size(HashMap *hash_map);

#endif  // KV_HASHMAP_H
