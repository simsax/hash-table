#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdbool.h>

#define DEBUG 1

typedef struct {
    uint32_t hash;
    uint32_t key_length;
    char* key; // owns the key (and has to free it)
    void* value; // should also own the value
} BucketStr;

typedef struct {
    uint32_t count;
    uint32_t capacity;
    BucketStr* buckets;
    uint32_t (*hash_func)(const char* key);
    uint32_t load_factor;
#if DEBUG
    int num_collisions;
#endif
} HashTableStr;

// hash functions
uint32_t FNV_1a(const char* key);

// table methods
void hashtable_str_init(HashTableStr* table, uint32_t (*hash_func)(const char* key));
void hashtable_str_free(HashTableStr* table);
bool hashtable_str_remove(HashTableStr* table, const char* key);
bool hashtable_str_get(HashTableStr* table, const char* key, void** value);
void hashtable_str_set(HashTableStr* table, const char* key, void* value);
void hashtable_str_print(HashTableStr* table);

#endif // TABLE_H
