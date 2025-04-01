#define  _GNU_SOURCE
#include "table.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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

    hashtable_str_set(&table, foo, &foo);
    hashtable_str_set(&table, bar, &bar);

    char* value = NULL;
    hashtable_str_get(&table, "foo", (void**)&value);
    value_print(value, "foo");

    value = NULL;
    hashtable_str_get(&table, "bar", (void**)&value);
    value_print(value, "bar");

    value = NULL;
    hashtable_str_get(&table, "baz", (void**)&value);
    value_print(value, "baz");

    char baz[] = "baz";
    value = NULL;
    hashtable_str_set(&table, baz, baz);
    hashtable_str_get(&table, "baz", (void**)&value);
    value_print(value, "baz");

    value = NULL;
    hashtable_str_remove(&table, baz);
    hashtable_str_get(&table, "baz", (void**)&value);
    value_print(value, "baz");

#if DEBUG
    printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
#endif

    hashtable_str_free(&table);
}

static void test_table_shakespeare(void) {
    HashTableStr table;
    hashtable_str_init(&table, NULL);

    FILE *fp = fopen("./shakespeare.txt", "r");
    if (!fp) {
       printf("ERROR: could not open file\n");
       exit(1);
    }

    char* line = NULL;
    long nread;
    size_t len = 0;
    while ((nread = getline(&line, &len, fp)) != -1) {
        hashtable_str_set(&table, line, line);
        char* value;
        bool found = hashtable_str_get(&table, line, (void**)&value);
        /*hashtable_str_print(&table);*/
#if DEBUG
        printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
#endif
        assert(found);
    }

    // read again, test that all the lines are in the hash table
    rewind(fp);
    while ((nread = getline(&line, &len, fp)) != -1) {
        char* value;
        bool found = hashtable_str_get(&table, line, (void**)&value);
        assert(found);
    }

#if DEBUG
    printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
#endif
    hashtable_str_free(&table);
}

// reads all file into memory, caller should free it
static char* read_all_file(const char* filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
       printf("ERROR: could not open file\n");
       exit(1);
    }
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    char* content = malloc(file_size + 1);
    if (content == NULL) {
        printf("Error: out of memory.\n");
        exit(1);
    }
    rewind(fp);
    size_t ret = fread(content, 1, file_size, fp);
    if (ret != file_size) {
        printf("Error: fread failed, ret = %zu\n", ret);
        exit(1);
    }
    return content;
}

static void test_str_processing(void) {
    // TODO: read entire file into memory
    // split by spaces
    char* file_content = read_all_file("./shakespeare.txt");
    printf("%s", file_content);
}

int main(void)
{
    // test_table_shakespeare();
    // test_table();
    test_str_processing();
    return 0;
}

