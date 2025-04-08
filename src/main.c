#include <stddef.h>
#define  _GNU_SOURCE
#include "table.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

static void value_print(char* value, const char* key) {
    if (value == NULL) {
        printf("%s not found\n", key);
    } else {
        printf("Key: %s, value: %s\n", key, value);
    }
}

// static void test_table(void) {
//     HashTableStr table;
//     hashtable_str_init(&table, NULL);
//
//     char foo[] = "foo";
//     char bar[] = "bar";
//
//     hashtable_str_set(&table, foo, &foo);
//     hashtable_str_set(&table, bar, &bar);
//
//     char* value = NULL;
//     hashtable_str_get(&table, "foo", (void**)&value);
//     value_print(value, "foo");
//
//     value = NULL;
//     hashtable_str_get(&table, "bar", (void**)&value);
//     value_print(value, "bar");
//
//     value = NULL;
//     hashtable_str_get(&table, "baz", (void**)&value);
//     value_print(value, "baz");
//
//     char baz[] = "baz";
//     value = NULL;
//     hashtable_str_set(&table, baz, baz);
//     hashtable_str_get(&table, "baz", (void**)&value);
//     value_print(value, "baz");
//
//     value = NULL;
//     hashtable_str_remove(&table, baz);
//     hashtable_str_get(&table, "baz", (void**)&value);
//     value_print(value, "baz");
//
// #if DEBUG
//     printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
// #endif
//
//     hashtable_str_free(&table);
// }
//
// static void test_table_shakespeare(void) {
//     HashTableStr table;
//     hashtable_str_init(&table, NULL);
//
//     FILE *fp = fopen("./shakespeare.txt", "r");
//     if (!fp) {
//        printf("ERROR: could not open file\n");
//        exit(1);
//     }
//
//     char* line = NULL;
//     long nread;
//     size_t len = 0;
//     while ((nread = getline(&line, &len, fp)) != -1) {
//         hashtable_str_set(&table, line, line);
//         char* value;
//         bool found = hashtable_str_get(&table, line, (void**)&value);
//         /*hashtable_str_print(&table);*/
// #if DEBUG
//         printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
// #endif
//         assert(found);
//     }
//
//     // read again, test that all the lines are in the hash table
//     rewind(fp);
//     while ((nread = getline(&line, &len, fp)) != -1) {
//         char* value;
//         bool found = hashtable_str_get(&table, line, (void**)&value);
//         assert(found);
//     }
//
// #if DEBUG
//     printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
// #endif
//     hashtable_str_free(&table);
// }

// reads all file into memory, caller should free it
static char* read_all_file(const char* filepath, size_t* file_length) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
       printf("ERROR: could not open file\n");
       exit(1);
    }
    fseek(fp, 0, SEEK_END);
    *file_length = ftell(fp);
    char* content = malloc(*file_length + 1);
    if (content == NULL) {
        printf("Error: out of memory.\n");
        exit(1);
    }
    rewind(fp);
    size_t ret = fread(content, 1, *file_length, fp);
    if (ret != *file_length) {
        printf("Error: fread failed, ret = %zu\n", ret);
        exit(1);
    }
    return content;
}

typedef struct {
    char* str;
    uint32_t length;
} TokenStr;

typedef struct {
    char* start;
    size_t file_length;
    size_t cur_ix;
} Tokenizer;

static void tokenizer_init(Tokenizer* tokenizer, char* source, size_t file_length) {
    tokenizer->start = source;
    tokenizer->cur_ix = 0;
    tokenizer->file_length = file_length;
}

static TokenStr tokenize_str(Tokenizer* tokenizer, const char* source) {
    if (tokenizer->cur_ix == tokenizer->file_length) {
        return (TokenStr) {
            .str = NULL,
            .length = 0
        };
    }
    uint32_t start_ix = tokenizer->cur_ix;
    while (!isspace(source[tokenizer->cur_ix]))
        tokenizer->cur_ix++;
    uint32_t end_ix = tokenizer->cur_ix;
    while (isspace(source[tokenizer->cur_ix]))
        tokenizer->cur_ix++;
    uint32_t len = end_ix - start_ix;
    char* buf = malloc(len + 1);
    memcpy(buf, &source[start_ix],len);
    return (TokenStr) {
        .str = buf,
        .length = len
    };
}

static void test_word_count(void) {
    size_t file_length = 0;
    char* content = read_all_file("./shakespeare.txt", &file_length);
    Tokenizer tokenizer;
    tokenizer_init(&tokenizer, content, file_length);
    HashTableStr table;
    hashtable_str_init(&table, NULL);

    // for (int i = 0; i < 100; i++) {
    for (;;) {
        TokenStr token = tokenize_str(&tokenizer, content);
        if (token.str == NULL) {
            break;
        }
        printf("%s\n", token.str);
        // printf("  %d => %s\n", i + 1, token.str);

        Value value = { .type = VAL_INT, .as.val_int = 0 };
        hashtable_str_get(&table, token.str, &value);
        value.as.val_int++;
        hashtable_str_set(&table, token.str, value);
    }

    // hashtable_str_print(&table);
}

int main(void)
{
    // test_table_shakespeare();
    // test_table();
    test_word_count();
    return 0;
}

