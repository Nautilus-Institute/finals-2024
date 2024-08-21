#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kv_list.h"
#include "sha256.h"


typedef struct KeyValueNode {
    char *key;
    void *data;
    size_t size;
    struct KeyValueNode *next;
} KeyValueNode;

static KeyValueNode *head = NULL;
static int volatile list_size = 0;

static KeyValueNode* list_find_node(const char *key)
{
    KeyValueNode *current = head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void list_store_data(const char *key, const void *data, size_t size)
{
#ifdef DEBUG
  printf("Key: %s\n", key);
#endif
  KeyValueNode *node = list_find_node(key);
  if (node != NULL) {
    free(node->data);
    node->data = malloc(size);
    memcpy(node->data, data, size);
    node->size = size;
  } else {
    node = (KeyValueNode*)malloc(sizeof(KeyValueNode));
    node->key = strdup(key);
    node->data = malloc(size);
    memcpy(node->data, data, size);
    node->size = size;
    node->next = head;
    head = node;
  }
  list_size ++;
}

ssize_t list_load_data(const char *key, void *buffer, size_t buffer_size)
{
  KeyValueNode *node = list_find_node(key);
  if (node != NULL) {
    if (buffer_size < node->size) {
        return -1; // Buffer too small
    }
    memcpy(buffer, node->data, node->size);
    return node->size;
  }
  return -1; // Key not found
}

void list_clear_db()
{
  KeyValueNode *current = head;
  while (current != NULL) {
    KeyValueNode *next = current->next;
    free(current->key);
    free(current->data);
    free(current);
    current = next;
  }
  head = NULL;
  list_size = 0;
}

int list_get_size()
{
  return list_size;
}

char* list_get_key(int index)
{
  KeyValueNode *current = head;
  while (current != NULL && index > 0) {
    current = current->next;
    index --;
  }
  if (current != NULL) {
    return current->key;
  }
  return NULL;
}

void* list_get_data(int index)
{
  KeyValueNode *current = head;
  while (current != NULL && index > 0) {
    current = current->next;
    index --;
  }
  if (current != NULL) {
    return current->data;
  }
  return NULL;
}

size_t list_get_data_size(int index)
{
  KeyValueNode *current = head;
  while (current != NULL && index > 0) {
    current = current->next;
    index --;
  }
  if (current != NULL) {
    return current->size;
  }
  return 0;
}

void digest_message(unsigned char *message, size_t size, char **digest, unsigned int *digest_len)
{
  const int out_size = 64;
  char* d = (char*)malloc(out_size);
  sha256sumstr(message, size, d);
  *digest = d;
  if (digest_len != NULL) {
    *digest_len = out_size;
  }
}

int is_valid_block(void *data, size_t size)
{
  char *sha256_1 = NULL, *sha256_2 = NULL; 

  if (size % 2 == 1) {
    return 0;
  }
  if (memcmp(data, data + size / 2, size / 2) == 0) {
    return 0;
  }

  // compute sha256 of the first half of the block
  int msg_size = size / 2;
  digest_message(data, msg_size, &sha256_1, NULL);

  // compute sha256 of the second half of the block
  digest_message(data + size - msg_size, msg_size, &sha256_2, NULL);

  if (memcmp(sha256_1, data + size - msg_size, 32) == 0) {
    return 1;
  }
  return 0;
}

void list_remove_bad_blocks()
{
  KeyValueNode *current = head;
  KeyValueNode *prev = NULL;
  while (current != NULL) {
    if (is_valid_block(current->data, current->size) == 0) {
      KeyValueNode *next = current->next;
      if (prev == NULL) {
        head = next;
      } else {
        prev->next = next;
      }
      free(current->key);
      free(current->data);
      free(current);
      current = next;
#ifdef DEBUG
      printf("Remove a bad block. %d remain.\n", list_size);
#endif
      list_size --;
    } else {
      prev = current;
      current = current->next;
    }
  }
}

