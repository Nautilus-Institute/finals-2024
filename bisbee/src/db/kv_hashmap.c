#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kv_hashmap.h"

void hh_create(HashMap *hash_map)
{
  hash_map->count = 0;
  memset(hash_map->table, 0, sizeof(hash_map->table));
}

unsigned int hash(const char *key)
{
  unsigned int hash = 5381;
  while (*key) {
    hash = ((hash << 5) + hash) + *key++;
  }
  return (hash % TABLE_SIZE) + 1;
}

void free_entry_data(Entry *entry)
{
  if (entry->size > 8) {
    free(entry->u.data);
  }
}

inline void store_entry_data(Entry *entry, const void *data, size_t size)
{
  if (size > 8) {
    entry->u.data = malloc(size);
    memcpy(entry->u.data, data, size);
  } else {
    memcpy(entry->u.short_data, data, size);
  }
  entry->size = size;
}

void hh_store_data(HashMap *hash_map, const char *key, const void *data, size_t size)
{
  unsigned int index = hash(key);
  Entry *entry = &hash_map->table[index];
#ifdef DEBUG
  if (index == TABLE_SIZE || 1) {
    printf("index = %d\n", index);
    printf("entry = %p\n", entry);
  }
#endif

  // Check if key already exists, update if so
  Entry *last_entry = entry;
  while (entry != NULL && entry->size != 0) {
    if (strcmp(entry->key, key) == 0) {
      free_entry_data(entry);
      store_entry_data(entry, data, size);
      hash_map->count++;
      return;
    }
    last_entry = entry;
    entry = entry->next;
  }

  // Key does not exist, create a new entry
  if (last_entry == &hash_map->table[index]) {
    // write to the table itself
    last_entry->key = strdup(key);
    store_entry_data(last_entry, data, size);
    last_entry->next = NULL;
    hash_map->count++;
    return;
  }

  entry = (Entry*)malloc(sizeof(Entry));
  entry->key = strdup(key);
  store_entry_data(entry, data, size);
  entry->next = NULL;
  last_entry->next = entry;
  hash_map->count++;
}

int hh_load_data(HashMap *hash_map, const char *key, void *buffer, size_t buffer_size)
{
  unsigned int index = hash(key);
  Entry *entry = &hash_map->table[index];

  while (entry != NULL && entry->size != 0) {
    if (strcmp(entry->key, key) == 0) {
      if (buffer_size < entry->size) {
        return -1; // Buffer too small
      }
      if (entry->size <= 8) {
        memcpy(buffer, entry->u.short_data, entry->size);
      } else {
        memcpy(buffer, entry->u.data, entry->size);
      }
      return entry->size;
    }
    entry = entry->next;
  }

  return -1; // Key not found
}

void hh_clear_db(HashMap *hash_map)
{
  for (int i = 0; i < TABLE_SIZE; i++) {
    Entry *entry = &hash_map->table[i];
    while (entry != NULL) {
      Entry *temp = entry;
      entry = entry->next;
      free(temp->key);
      if (temp->size > 8) {
        free(temp->u.data);
      }
      if (temp != &hash_map->table[i]) {
        // freeable
        free(temp);
      }
    }
    hash_map->table[i].size = 0;
  }
  hash_map->count = 0;
}

void free_hash_map(HashMap *hash_map)
{
  hh_clear_db(hash_map);
}

int hh_get_size(HashMap* hash_map)
{
  return hash_map->count;
}

