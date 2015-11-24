/*
 * measurements.c
 *
 * Maintains the set of current measurements
 * Simple key/value store
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <assert.h>

#include "ezxml.h"

#include "measurements.h"


// In memory database
static ezxml_t store;


// Loads previously written measurements, 
// or inits a new measurement store
static void init()
{
    // already initialized
    if (store != NULL)
        return;

    FILE *f = fopen(DATABASE, "r");
    if (f != NULL)
    {
        // load previous version
        store = ezxml_parse_fp(f);
        fclose(f);
        if (store == NULL)
            perror(ezxml_error(store));
        else
            return;

    }

    store = ezxml_new("status");
}


// Upserts a measurement value
void measurement_set(const char *key, const char *value)
{
    init();

    // silently ignore empty keys
    if (*key == '\0')
        return;

    // find existing child
    ezxml_t child = ezxml_child(store, key);

    if (child == NULL)
    {
        // add new key
        child = ezxml_add_child_d(store, key, 0);
    }

    ezxml_set_txt_d(child, value);
}

// returns the current value of the given key
// or an empty string if the key was not found
const char * measurement_get(const char *key)
{
    init();

    ezxml_t child = ezxml_child(store, key);

    if (child == NULL)
        return "";
    else
        return ezxml_txt(child);

}


// Get all keys as a \0\0-terminated list of strings
// Caller should free()
char * measurement_getkeys()
{
    init();

    int len=1; 
    char *result = malloc(1);
    char *p = result;

    ezxml_t child = store->child;
    while(child != NULL)
    {
        len += strlen(child->name)+1;
        result = realloc(result, len);
        strcpy(p, child->name);
        p += strlen(child->name) + 1; // move p to after \0
        child = child->sibling;
    }
    *p = '\0';
    return result;
}

// Stores the current measurement state to disk
void measurement_write()
{
    // try writing to new file
    FILE *fnew = fopen(DATABASE ".new", "w");
    if (!fnew)
    {
        perror("Can't write database");
        return;
    }

    // convert to string & write
    char * xml = ezxml_toxml(store);
    int result = fputs(xml, fnew);

    free(xml);
    fclose(fnew);

    if (result != EOF) // no error?
    {
        // we use rename-overwrite to ensure
        // atomic write
        rename(DATABASE ".new", DATABASE);
    }
}


#ifdef TEST

static void test_getset()
{
    // fresh start
    unlink(DATABASE);
    store = NULL;

    measurement_set("wok", "bar");
    measurement_set("foo", "3.0");
    measurement_set("foo", "2.0");

    assert(strcmp(measurement_get("wok"), "bar") == 0);
    assert(strcmp(measurement_get("foo"), "2.0") == 0);

}

static void test_persist()
{
    // fresh start
    unlink(DATABASE);
    store = NULL;

    measurement_set("wok", "baz");
    measurement_write();
    measurement_write();

    // force reload
    store = NULL;


    // should get data from disk
    assert(strcmp(measurement_get("wok"), "baz") == 0);
}

static void test_failpersist()
{
    // fresh start
    unlink(DATABASE);
    store = NULL;

    measurement_set("wok", "baz");
    measurement_write();

    // mess up template file
    FILE *f = fopen(DATABASE, "w");
    fputs("ugh", f);
    fclose(f);

    // force reload
    store = NULL;

    // should be empty
    assert(strcmp(measurement_get("wok"), "") == 0);
}


static void test_getall()
{
    // fresh start
    unlink(DATABASE);
    store = NULL;

    measurement_set("wok", "baz");
    measurement_set("wor", "baz");
    measurement_set("bop", "baz");

    char *res = measurement_getkeys();

    // note we're testing fixed order, but this is not necessary
    assert(memcmp(res, "wok\0wor\0bop\0\0", (3*4)+1) ==0);
    
    free(res);
}


int main(void) {

    test_getset();
    test_failpersist();
    test_persist();
    test_getall();

    puts("Measurement tests OK");

    return 0;
}

#endif

