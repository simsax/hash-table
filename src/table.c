#include "table.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: custom allocator
#define FREE(x) free(x)
#define CALLOC(capacity, elemsize) calloc((capacity), (elemsize))
#define CALC_LOAD_FACTOR(table) (int)(((float)((table)->count + 1) / (table)->capacity) * 100)


static char* _strdup(const char* str, size_t length) {
    int size = length + 1;
    char* new_str = malloc(size);
    memcpy(new_str, str, size);
    return new_str;
}

// uint32_t FNV_1a(const char* key) {
//     uint32_t hash = 2166136261u;
//     int length = strlen(key);
//     for (int i = 0; i < length; i++) {
//         hash ^= (uint8_t)key[i];
//         hash *= 16777619;
//     }
//     return hash;
// }

uint32_t FNV_1a(const char* key, size_t key_length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < key_length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

static void hashtable_str_grow(HashTableStr* table) {
    uint32_t old_capacity = table->capacity;
    if (table->capacity == 0) {
        table->capacity = 8;
    } else {
        table->capacity *= 2;
    }
    BucketStr* old_buckets = table->buckets;
    table->buckets = CALLOC(table->capacity, sizeof *table->buckets);
    if (table->buckets == NULL) {
        printf("ERROR: out of memory, aborting.\n");
        exit(1);
    }

    // create new table from scratch (copy old elements into new table)
    table->count = 0;
    for (uint32_t i = 0; i < old_capacity; i++) {
        BucketStr* bucket = &old_buckets[i];
        if (bucket->key != NULL) {
            hashtable_str_set(table, bucket->key, bucket->key_length, bucket->value);
        }
    }

    FREE(old_buckets);
}

void hashtable_str_init(HashTableStr* table, uint32_t (*hash_func)(const char* key, size_t key_length)) {
    table->count = 0;
    table->capacity = 0;
    table->buckets = NULL;
    table->load_factor = 70;
    if (hash_func == NULL) {
        // default hash function
        table->hash_func = &FNV_1a;
    } else {
        table->hash_func = hash_func;
    }
    hashtable_str_grow(table);
#if DEBUG
    table->num_collisions = 0;
#endif
}

void hashtable_str_free(HashTableStr* table) {
    FREE(table->buckets);
}

static BucketStr* hashtable_str_find(HashTableStr* table, const char* key, uint32_t hash, uint32_t length) {
    uint32_t index = hash & (table->capacity - 1); // mod of 2^n is equal to the last n bits

    int num_iterations = 0;
    for (;;) {
        BucketStr* tombstone = NULL;
        BucketStr* bucket = &table->buckets[index];

        // printf("Before check\n");
        if (bucket->key == NULL) {
            if (bucket->value.as.val_void == NULL) {
                return tombstone == NULL ? bucket : tombstone;
            } else {
                if (tombstone == NULL) tombstone = bucket;
            }
        } else if (
                bucket->key_length == length &&
                bucket->hash == hash &&
                memcmp(bucket->key, key, length) == 0
                ) {
            return bucket;
        }

#if DEBUG
        table->num_collisions++;
#endif

        // linear probing
        index += 1;
        // index = (index + 1) & (table->capacity - 1);

        // quadratic probing
        // index += 1 << num_iterations++;
        index &= (table->capacity - 1);
    }
}

bool hashtable_str_remove(HashTableStr* table, const char* key, size_t key_length) {
    if (table->count == 0) 
        return false;
    uint32_t hash = table->hash_func(key, key_length);
    BucketStr* bucket = hashtable_str_find(table, key, hash, key_length);
    if (bucket->key == NULL)
        return false;

    // place tombstone
    bucket->key = NULL;
    bucket->value.as.val_int = 1;
    return true;
}

bool hashtable_str_get(HashTableStr* table, const char* key, size_t key_length, Value* value) {
    if (table->count == 0)
        return false;
    uint32_t hash = table->hash_func(key, key_length);
    BucketStr* bucket = hashtable_str_find(table, key, hash, key_length);
    if (bucket->key == NULL)
        return false;
    *value = bucket->value;
    return true;
}

void hashtable_str_set(HashTableStr* table, const char* key, size_t key_length, Value value) {
    if (CALC_LOAD_FACTOR(table) >= table->load_factor) {
        hashtable_str_grow(table);
    }

    uint32_t hash = table->hash_func(key, key_length);
    BucketStr* bucket = hashtable_str_find(table, key, hash, key_length);

    if (bucket->key == NULL && bucket->value.as.val_void == NULL) {
        // new item
        table->count++;
    }
    
    // insert/set new value
    bucket->key = key;
    // bucket->key = _strdup(key, key_length);
    bucket->hash = hash;
    bucket->key_length = key_length;
    bucket->value = value;
}

void print_key(const char* key, size_t length) {
    for (size_t i = 0; i < length; i++) {
        putchar(key[i]);
    }
}

void hashtable_str_print(HashTableStr* table) {
    printf("===== TABLE =====\n");
    for (uint32_t i = 0; i < table->capacity; i++) {
        BucketStr* bucket = &table->buckets[i];
        if (bucket->key != NULL) {
            print_key(bucket->key, bucket->key_length);
            if (bucket->value.type == VAL_INT) {
                printf(": %d\n", bucket->value.as.val_int);
            } else {
                // assume it's a char pointer
                printf(": %s\n", (const char*)bucket->value.as.val_void);
            }
        } else {
            if (bucket->value.as.val_void == NULL) {
                printf("NULL\n");
            } else {
                printf("[Tombstone]\n");
            }
        }
    }
    printf("=================\n");
}

void hashtable_sort(HashTableStr* table, int (*compare)(const void* b1, const void* b2)) {
    qsort(table->buckets, table->capacity, sizeof *table->buckets, compare);
}
