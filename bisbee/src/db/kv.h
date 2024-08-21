#ifndef KV_H 
#define KV_H

#include <stddef.h>
#include <sys/types.h>

// Stores data with a specific key
void store_data(const char *key, const void *data, size_t size);

// Loads data for a specific key into the provided buffer
// Returns the size of the data, or -1 if the key is not found
ssize_t load_data(const char *key, void *buffer, size_t buffer_size);

// Clears all data in the database
void clear_db();

void initialize();

#endif // KV_H
