#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

#define DEBUG 1

typedef struct {
    /*uint32_t hash;*/
    char* key; // owns the key (and has to free it)
    const void* value;
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
void hashtable_str_insert(HashTableStr* table, const char* key, const void* value);
BucketStr* hashtable_str_find(HashTableStr* table, const char* key);
void hashtable_str_remove(HashTableStr* table, const char* key);
void hashtable_str_print(HashTableStr* table);

#endif // TABLE_H
