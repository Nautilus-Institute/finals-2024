#ifndef KV_LIST_H 
#define KV_LIST_H

#include <stddef.h>
#include <sys/types.h>

// Stores data with a specific key
void list_store_data(const char *key, const void *data, size_t size);

// Loads data for a specific key into the provided buffer
// Returns the size of the data, or -1 if the key is not found
ssize_t list_load_data(const char *key, void *buffer, size_t buffer_size);

// Clears all data in the database
void list_clear_db();

int list_get_size();

char* list_get_key(int index);

void* list_get_data(int index);

size_t list_get_data_size(int index);

#endif // KV_LIST_H
