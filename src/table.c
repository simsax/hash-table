#define  _GNU_SOURCE
#include "table.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t FNV_1a(const char* key) {
    uint32_t hash = 2166136261u;
    int length = strlen(key);
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

static void hashtable_str_grow(HashTableStr* table) {
    uint32_t old_capacity = table->capacity;
    if (table->capacity == 0) {
        table->capacity = 16;
    } else {
        table->capacity *= 2;
    }
    BucketStr* old_buckets = table->buckets;
    table->buckets = calloc(table->capacity, sizeof *table->buckets);
    if (table->buckets == NULL) {
        printf("ERROR: out of memory, aborting.\n");
        exit(1);
    }

    /*for (uint32_t i = 0; i < table->capacity; i++) {*/
    /*    new_buckets[i].key = NULL;*/
    /*    new_buckets[i].value = NULL;*/
    /*}*/

    // create new table from scratch (copy old elements into new table)
    for (uint32_t i = 0; i < old_capacity; i++) {
        BucketStr* bucket = &old_buckets[i];
        if (bucket->key != NULL) {
            hashtable_str_insert(table, bucket->key, bucket->value);
            free(bucket->key); // TODO: better way
        }
    }

    free(old_buckets);
}

void hashtable_str_init(HashTableStr* table, uint32_t (*hash_func)(const char* key)) {
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
}

void hashtable_str_free(HashTableStr* table) {
    for (uint32_t i = 0; i < table->capacity; i++) {
        BucketStr* bucket = &table->buckets[i];
        if (table->buckets[i].key != NULL)
            free(bucket->key);
    }
    free(table->buckets);
}

BucketStr* hashtable_str_find(HashTableStr* table, const char* key) {
    uint32_t hash = table->hash_func(key);
    uint32_t index = hash & (table->capacity - 1); // works for capacity which is a power of 2

    while (table->buckets[index].key != NULL && strcmp(table->buckets[index].key, key) != 0) {
        index = (index + 1) & (table->capacity - 1);
    }

    BucketStr* bucket = &table->buckets[index];
    if (bucket->value == NULL)
        return NULL;

    return bucket;
}

void hashtable_str_insert(HashTableStr* table, const char* key, const void* value) {
    if (((table->count + 1) / table->capacity) * 100 >= table->load_factor) {
        // expand table
        hashtable_str_grow(table);
    }

    uint32_t hash = table->hash_func(key);
    uint32_t index = hash & (table->capacity - 1);

    while (table->buckets[index].key != NULL && table->buckets[index].value != NULL && strcmp(table->buckets[index].key, key) != 0) {
        index = (index + 1) & (table->capacity - 1);
#if DEBUG
        table->num_collisions++;
#endif
    }

    if (table->buckets[index].key == NULL) {
        // new item
        table->count++;
    }
    BucketStr* bucket = &table->buckets[index];
    
    // insert/set new value
    /*bucket->hash = hash;*/
    bucket->key = strdup(key);
    bucket->value = value;
}

void hashtable_str_remove(HashTableStr* table, const char* key) {
    BucketStr* bucket = hashtable_str_find(table, key);
    if (bucket != NULL) {
        bucket->value = NULL;
    }
}

void hashtable_str_print(HashTableStr* table) {
    printf("===== TABLE =====\n");
    for (uint32_t i = 0; i < table->capacity; i++) {
        BucketStr* bucket = &table->buckets[i];
        if (bucket->key != NULL && bucket->value != NULL)
            printf("%s: %s\n", bucket->key, (const char*)bucket->value);
    }
    printf("=================\n");
}
