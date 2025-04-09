#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define DEBUG 1

// typedef union {
// } KeyType;

typedef enum {
    VAL_VOID,
    VAL_INT
} ValueType;

typedef struct {
    ValueType type;
    union {
        void* val_void;
        int val_int;
    } as;
} Value;

typedef struct {
    uint32_t hash;
    size_t key_length;
    const char* key;
    Value value;
} BucketStr;

typedef struct {
    uint32_t count;
    uint32_t capacity;
    BucketStr* buckets;
    uint32_t (*hash_func)(const char* key, size_t key_length);
    uint32_t load_factor;
#if DEBUG
    int num_collisions;
#endif
} HashTableStr;

// hash functions
uint32_t FNV_1a(const char* key, size_t key_length);

// table methods
void hashtable_str_init(HashTableStr* table, uint32_t (*hash_func)(const char* key, size_t key_length));
void hashtable_str_free(HashTableStr* table);
bool hashtable_str_remove(HashTableStr* table, const char* key, size_t key_length);
bool hashtable_str_get(HashTableStr* table, const char* key, size_t key_length, Value* value);
void hashtable_str_set(HashTableStr* table, const char* key, size_t key_length, Value value);

// debug
void hashtable_str_print(HashTableStr* table);
void hashtable_sort(HashTableStr* table, int (*compare)(const void* b1, const void* b2));
void print_key(const char* key, size_t length);

#endif // TABLE_H
