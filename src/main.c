#include <stddef.h>
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

static void test_table(void) {
    HashTableStr table;
    hashtable_str_init(&table, NULL);

    char foo[] = "foo";
    char bar[] = "bar";

    hashtable_str_set(&table, foo, 3, (Value) {.type=VAL_VOID, .as.val_void=foo});
    hashtable_str_set(&table, bar, 3, (Value) {.type=VAL_VOID, .as.val_void=bar});

    Value val = {.type=VAL_VOID, .as.val_void=NULL};
    hashtable_str_get(&table, "foo", 3, &val);
    value_print(val.as.val_void, "foo");

    val = (Value){.type=VAL_VOID, .as.val_void=NULL};
    hashtable_str_get(&table, "bar", 3, &val);
    value_print(val.as.val_void, "bar");

    val = (Value){.type=VAL_VOID, .as.val_void=NULL};
    hashtable_str_get(&table, "baz", 3, &val);
    value_print(val.as.val_void, "baz");

    char baz[] = "baz";
    hashtable_str_set(&table, baz, 3, (Value) {.type=VAL_VOID, .as.val_void=baz});
    val = (Value){.type=VAL_VOID, .as.val_void=NULL};
    hashtable_str_get(&table, "baz", 3, &val);
    value_print(val.as.val_void, "baz");

    val = (Value){.type=VAL_VOID, .as.val_void=NULL};
    hashtable_str_remove(&table, baz, 3);
    hashtable_str_get(&table, "baz", 3, &val);
    value_print(val.as.val_void, "baz");

#if DEBUG
    printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
#endif

    hashtable_str_print(&table);
    hashtable_str_free(&table);
}

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
    const char* start;
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
            .start = NULL,
            .length = 0
        };
    }
    // if any whitespace at beginning, remove it
    while (isspace(source[tokenizer->cur_ix]))
        tokenizer->cur_ix++;

    uint32_t start_ix = tokenizer->cur_ix;
    const char* start = &source[start_ix];

    // eat the token
    while (!isspace(source[tokenizer->cur_ix]))
        tokenizer->cur_ix++;
    uint32_t end_ix = tokenizer->cur_ix;

    // trim all whitespaces before the end
    while (isspace(source[tokenizer->cur_ix]))
        tokenizer->cur_ix++;

    uint32_t len = end_ix - start_ix;
    return (TokenStr) {
        .start = start,
        .length = len
    };
}

static void print_token(TokenStr token) {
    for (size_t i = 0; i < token.length; i++) {
        putchar(token.start[i]);
    }
}

static int compare_descending(const void* b1, const void* b2) {
    return ((const BucketStr*)b2)->value.as.val_int - ((const BucketStr*)b1)->value.as.val_int;
}

static int compare_key(const void* b1, const void* b2) {
    const BucketStr* b1_str = (const BucketStr*)b1;
    const BucketStr* b2_str = (const BucketStr*)b2;

    if (b1_str->key == NULL && b2_str->key == NULL)
        return 0;
    else if (b1_str->key == NULL)
        return 1;
    else
        return -1;

    return strncmp(b2_str->key, b1_str->key, b1_str->key_length);
}

static void test_word_count(void) {
    size_t file_length = 0;
    char* content = read_all_file("./shakespeare.txt", &file_length);
    Tokenizer tokenizer;
    tokenizer_init(&tokenizer, content, file_length);
    HashTableStr table;
    hashtable_str_init(&table, NULL);

    for (;;) {
        TokenStr token = tokenize_str(&tokenizer, content);
        if (token.start == NULL) {
            break;
        }

        Value value = { .type = VAL_INT, .as.val_int = 0 };
        hashtable_str_get(&table, token.start, token.length, &value);
        value.as.val_int++;
        hashtable_str_set(&table, token.start, token.length, value);
    }

    // num of keys
    int word_count = 0;
    for (size_t i = 0; i < table.capacity; i++) {
        if (table.buckets[i].key != NULL) {
            word_count++;
        }
    }
    printf("%d unique words\n", word_count);

    // print top 20 items
    hashtable_sort(&table, compare_descending);
    size_t max_tops = 100;
    for (size_t i = 0; i < max_tops && i < table.capacity; i++) {
        BucketStr* bucket = &table.buckets[i];
        if (bucket->key != NULL) {
            print_key(bucket->key, bucket->key_length);
            if (bucket->value.type == VAL_INT) {
                printf(": %d\n", bucket->value.as.val_int);
            }
        }
    }

    free(content);
}

int main(void)
{
    // test_table_shakespeare();
    // test_table();
    test_word_count();
    return 0;
}

