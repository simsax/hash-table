#include "table.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FREE(x) free(x)
#define CALLOC(capacity, elemsize) calloc((capacity), (elemsize))
#define CALC_LOAD_FACTOR(table) (int)(((float)((table)->count + 1) / (table)->capacity) * 100)


static char* _strdup(const char* str, size_t length) {
    int size = length + 1;
    char* new_str = malloc(size);
    memcpy(new_str, str, size);
    return new_str;
}

uint32_t FNV_1a(const char* key, size_t key_length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < key_length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

uint32_t hash_int32(int key) {
    uint32_t hash = ((key >> 16) ^ key) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    return hash;
}

uint64_t hash_int64(int key) {
    uint64_t hash = key ^ (key >> 30);
    hash *= 0xbf58476d1ce4e5b9U;
    hash ^= hash >> 27;
    hash *= 0x94d049bb133111ebU;
    hash ^= hash >> 31;
    return hash;
}

static void ht_str_grow(HashTableStr* table) {
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
            ht_str_set(table, bucket->key, bucket->key_length, bucket->value);
        }
    }

    FREE(old_buckets);
}

void ht_str_init(HashTableStr* table, uint32_t size, uint32_t load_factor, uint32_t (*hash_func)(const char* key, size_t key_length)) {
    table->count = 0;
    table->buckets = NULL;
    table->load_factor = load_factor;
    if (hash_func == NULL) {
        // default hash function
        table->hash_func = FNV_1a;
    } else {
        table->hash_func = hash_func;
    }

    // init table
    table->capacity = size;
    table->buckets = CALLOC(table->capacity, sizeof *table->buckets);
    if (table->buckets == NULL) {
        printf("ERROR: out of memory, aborting.\n");
        exit(1);
    }

#if DEBUG
    table->num_collisions = 0;
#endif
}

void ht_str_free(HashTableStr* table) {
    FREE(table->buckets);
}

static BucketStr* ht_str_find(HashTableStr* table, const char* key, uint32_t hash, uint32_t length) {
    uint32_t index = hash & (table->capacity - 1); // mod of 2^n is equal to the last n bits

    int num_iterations = 0;
    BucketStr* tombstone = NULL;
    for (;;) {
        BucketStr* bucket = &table->buckets[index];

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
        /*index += 1 << num_iterations++;*/
        index &= (table->capacity - 1);
    }
}

bool ht_str_remove(HashTableStr* table, const char* key, size_t key_length) {
    if (table->count == 0) 
        return false;
    uint32_t hash = table->hash_func(key, key_length);
    BucketStr* bucket = ht_str_find(table, key, hash, key_length);
    if (bucket->key == NULL)
        return false;

    // place tombstone
    bucket->key = NULL;
    bucket->value.as.val_int = 1;
    return true;
}

bool ht_str_get(HashTableStr* table, const char* key, size_t key_length, Value* value) {
    if (table->count == 0)
        return false;
    uint32_t hash = table->hash_func(key, key_length);
    BucketStr* bucket = ht_str_find(table, key, hash, key_length);
    if (bucket->key == NULL)
        return false;
    *value = bucket->value;
    return true;
}

void ht_str_set(HashTableStr* table, const char* key, size_t key_length, Value value) {
    if (CALC_LOAD_FACTOR(table) >= table->load_factor) {
        ht_str_grow(table);
    }

    uint32_t hash = table->hash_func(key, key_length);
    BucketStr* bucket = ht_str_find(table, key, hash, key_length);

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

void ht_str_print(HashTableStr* table) {
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

void ht_str_sort(HashTableStr* table, int (*compare)(const void* b1, const void* b2)) {
    qsort(table->buckets, table->capacity, sizeof *table->buckets, compare);
}


// int implementation

static void ht_int_grow(HashTableInt* table) {
    uint32_t old_capacity = table->capacity;
    if (table->capacity == 0) {
        table->capacity = 8;
    } else {
        table->capacity *= 2;
    }
    BucketInt* old_buckets = table->buckets;
    table->buckets = CALLOC(table->capacity, sizeof *table->buckets);
    if (table->buckets == NULL) {
        printf("ERROR: out of memory, aborting.\n");
        exit(1);
    }

    // create new table from scratch (copy old elements into new table)
    table->count = 0;
    for (uint32_t i = 0; i < old_capacity; i++) {
        BucketInt* bucket = &old_buckets[i];
        if (bucket->occupied) {
            ht_int_set(table, bucket->key, bucket->value);
        }
    }

    FREE(old_buckets);
}

void ht_int_init(HashTableInt* table, size_t size, uint32_t load_factor, uint64_t (*hash_func)(int key)) {
    table->count = 0;
    table->buckets = NULL;
    table->load_factor = load_factor;
    table->first_collision = false;
    if (hash_func == NULL) {
        // default hash function
        table->hash_func = hash_int64;
    } else {
        table->hash_func = hash_func;
    }

    // init table
    table->capacity = size;
    table->buckets = CALLOC(table->capacity, sizeof *table->buckets);
    if (table->buckets == NULL) {
        printf("ERROR: out of memory, aborting.\n");
        exit(1);
    }
#if DEBUG
    table->num_collisions = 0;
#endif
}

void ht_int_free(HashTableInt* table) {
    FREE(table->buckets);
}

static BucketInt* ht_int_find(HashTableInt* table, int key, uint32_t hash) {
    uint32_t index = hash & (table->capacity - 1); // mod of 2^n is equal to the last n bits

    int num_iterations = 0;
    BucketInt* tombstone = NULL;
    for (;;) {
        BucketInt* bucket = &table->buckets[index];

        if (!bucket->occupied) {
            if (bucket->value.as.val_void == NULL) {
                return tombstone == NULL ? bucket : tombstone;
            } else {
                if (tombstone == NULL) tombstone = bucket;
            }
        } else if (bucket->key == key) {
            return bucket;
        }

#if DEBUG
        table->num_collisions++;
        /*if (!table->first_collision) {*/
        /*    table->first_collision = true;*/
        /*    printf("First collision happened at entry: %zu | capacity / no_collision_entries: %f \n", table->count, (double)table->capacity / table->count);*/
        /*}*/
#endif

        // linear probing
        index += 1;

        // quadratic probing
        /*index += 1 << num_iterations++;*/
        index &= (table->capacity - 1);
    }
}

bool ht_int_remove(HashTableInt* table, int key) {
    if (table->count == 0) 
        return false;
    uint32_t hash = table->hash_func(key);
    BucketInt* bucket = ht_int_find(table, key, hash);
    if (!bucket->occupied)
        return false;

    // place tombstone
    bucket->occupied = false;
    bucket->value.as.val_int = 1;
    return true;
}

bool ht_int_get(HashTableInt* table, int key, Value* value) {
    if (table->count == 0)
        return false;
    uint32_t hash = table->hash_func(key);
    BucketInt* bucket = ht_int_find(table, key, hash);
    if (!bucket->occupied)
        return false;
    *value = bucket->value;
    return true;
}

void ht_int_set(HashTableInt* table, int key, Value value) {
    if (CALC_LOAD_FACTOR(table) >= table->load_factor) {
        ht_int_grow(table);
    }

    uint32_t hash = table->hash_func(key);
    BucketInt* bucket = ht_int_find(table, key, hash);

    if (!bucket->occupied && bucket->value.as.val_void == NULL) {
        // new item
        table->count++;
    }
    
    // insert/set new value
    bucket->key = key;
    bucket->value = value;
    bucket->occupied = true;
}

void ht_int_print(HashTableInt* table) {
    printf("===== TABLE =====\n");
    for (uint32_t i = 0; i < table->capacity; i++) {
        BucketInt* bucket = &table->buckets[i];
        if (bucket->occupied) {
            printf("%d: %d\n", bucket->key, bucket->value.as.val_int);
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

