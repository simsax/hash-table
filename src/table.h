#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define DEBUG 1

typedef enum {
    VAL_VOID,
    VAL_INT,
    VAL_FLOAT,
    VAL_DOUBLE
} ValueType;

typedef struct {
    ValueType type;
    union {
        void* val_void;
        int val_int;
        float val_float;
        double val_double;
    } as;
} Value;

typedef struct {
    uint32_t hash;
    size_t key_length;
    const char* key;
    Value value;
} BucketStr;

typedef struct {
    int key;
    bool occupied; // can't represent an invalid value with an int
    Value value;
} BucketInt;

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

typedef struct {
    size_t count;
    size_t capacity;
    BucketInt* buckets;
    uint64_t (*hash_func)(int key);
    uint32_t load_factor;
#if DEBUG
    size_t num_collisions;
    bool first_collision; // birthday paradox test
#endif
} HashTableInt;

// hash functions
uint32_t FNV_1a(const char* key, size_t key_length);
uint32_t hash_int32(int key);
uint64_t hash_int64(int key);

// table methods
void ht_str_init(HashTableStr* table, uint32_t size, uint32_t load_factor, uint32_t (*hash_func)(const char* key, size_t key_length));
void ht_str_free(HashTableStr* table);
bool ht_str_remove(HashTableStr* table, const char* key, size_t key_length);
bool ht_str_get(HashTableStr* table, const char* key, size_t key_length, Value* value);
void ht_str_set(HashTableStr* table, const char* key, size_t key_length, Value value);

// debug
void ht_str_print(HashTableStr* table);
void ht_str_sort(HashTableStr* table, int (*compare)(const void* b1, const void* b2));
void print_key(const char* key, size_t length);

// int key table (for now I keep them separate because I don't wanna deal with void pointers in the key
void ht_int_init(HashTableInt* table, size_t size, uint32_t load_factor, uint64_t (*hash_func)(int key));
void ht_int_free(HashTableInt* table);
bool ht_int_remove(HashTableInt* table, int key);
bool ht_int_get(HashTableInt* table, int key, Value* value);
void ht_int_set(HashTableInt* table, int key, Value value);

// debug
void ht_int_print(HashTableInt* table);
void ht_int_sort(HashTableInt* table, int (*compare)(const void* b1, const void* b2));

#endif // TABLE_H
