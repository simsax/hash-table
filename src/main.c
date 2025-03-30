#define  _GNU_SOURCE
#include "table.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void bucket_print(BucketStr* bucket, const char* key) {
    if (bucket == NULL) {
        printf("%s not found\n", key);
    } else {
        printf("Key: %s, value: %s\n", bucket->key, (const char*)bucket->value);
    }

}

static void test_table(void) {
    HashTableStr table;
    hashtable_str_init(&table, NULL);

    const char* foo = "foo";
    const char* bar = "bar";

    hashtable_str_insert(&table, foo, foo);
    hashtable_str_insert(&table, bar, bar);

    BucketStr* bucket = hashtable_str_find(&table, "foo");
    bucket_print(bucket, "foo");

    bucket = hashtable_str_find(&table, "bar");
    bucket_print(bucket, "bar");

    bucket = hashtable_str_find(&table, "baz");
    bucket_print(bucket, "baz");

    const char* baz = "baz";

    hashtable_str_insert(&table, baz, baz);
    bucket = hashtable_str_find(&table, "baz");
    bucket_print(bucket, "baz");

    hashtable_str_remove(&table, baz);
    bucket = hashtable_str_find(&table, "baz");
    bucket_print(bucket, "baz");

#if DEBUG
    printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
#endif
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
        hashtable_str_insert(&table, line, line);
        BucketStr* bucket = hashtable_str_find(&table, line);
        /*hashtable_str_print(&table);*/
#if DEBUG
        printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
#endif
        assert(bucket != NULL);
    }

    // read again, test that all the lines are in the hash table
    rewind(fp);
    while ((nread = getline(&line, &len, fp)) != -1) {
        BucketStr* bucket = hashtable_str_find(&table, line);
        assert(bucket != NULL);
    }

#if DEBUG
    printf("\nCount: %d\nCapacity: %d\nLoad: %d%%\nNum collisions: %d\n", table.count, table.capacity, (int)(table.count * 100 / (float) table.capacity), table.num_collisions);
#endif
}

int main(void)
{
    test_table_shakespeare();
    /*test_table();*/
    return 0;
}

