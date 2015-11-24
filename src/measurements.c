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
// an array of key-value pairs
// each element is "key\0value\0"
char **store = NULL;
int store_len = 0;


// Loads previously written measurements, 
// or inits a new measurement store
static void init()
{
    // already initialized?
    if (store != NULL)
        return;

    store = malloc(0);
    store_len = 0;

    FILE *f = fopen(DATABASE, "r");
    if (f != NULL)
    {
        // load previous version
        puts("Loading data from " DATABASE);

        ezxml_t xml = ezxml_parse_fp(f);
        const char *result = ezxml_error(xml);
        fclose(f);

        if (store == NULL || *result != '\0')
            perror(result);
        else
        {
            ezxml_t child = xml->child;
            while(child != NULL)
            {
                // key = tagname, value=text content
                measurement_set(child->name, ezxml_txt(child));

                child = child->sibling;
            }
        }
        ezxml_free(xml);

    }

}

// wrapper for key-compare
int compare(const void *s1, const void *s2)
{
    return (strcmp(*(char **)s1, *(char **)s2));
}

// Upserts a measurement value
void measurement_set(const char *key, const char *value)
{
    init();

    // silently ignore empty keys
    if (*key == '\0')
        return;

    int keylen = strlen(key);
    int vallen = strlen(value);

    // find existing key
    char **result = bsearch(&key, store, store_len,
            sizeof(char*), compare);

    if (result)
    {
        // update existing
        
        // find old value, and make space
        char *oldval = *result + keylen + 1;
        if (strlen(oldval) < vallen)
            *result = realloc(*result, (keylen+vallen+2)*sizeof(char*));

        // set value after key
        strcpy((*result) + keylen+1, value);
    }
    else
    {
        // insert new; uses sorted insert

        //make space
        store_len++;
        store = realloc(store, sizeof(char*) * store_len);

        // find spot for insertion
        int n;
        for(n=0; n< store_len-1; n++)
        {
            if (strcmp(key, store[n]) < 0)
                break;
        }

        // shift everything from n one down
        memmove(store + n + 1, store + n,
            sizeof(char*) * (store_len-n-1));

        store[n] = malloc((keylen + vallen + 2)* sizeof(char*));
        strcpy(store[n], key);
        
        // set value after key
        strcpy(store[n] + keylen+1, value);

    }
}

// returns the current value of the given key
// or an empty string if the key was not found
const char * measurement_get(const char *key)
{
    init();

    // find existing child
    char **result = bsearch(&key, store, store_len,
            sizeof(char*), compare);

    if (result==NULL)
        return "";
    else
        // return value stored after key
        return (*result)+ strlen(*result) + 1;
}


// Get all keys as an array of strings
// Stores the count in the passed variable
const char ** measurement_getkeys(int *count)
{
    init();
    *count = store_len;
    return (const char**)store; 
}

// Stores the current measurement state to disk
// We use the same xml format as requests
void measurement_write()
{
    // try writing to new file
    FILE *fnew = fopen(DATABASE ".new", "w");
    if (!fnew)
    {
        perror("Can't write database");
        return;
    }

    // write xml
    fputs("<status>\n",fnew);
    for(int n=0; n < store_len; n++)
    {
        // construct xml for this key/val
        ezxml_t val_xml = ezxml_new(store[n]);
        ezxml_set_txt(val_xml, store[n] + strlen(store[n])+1);
        char *t = ezxml_toxml(val_xml);

        fprintf(fnew, "  %s\n", t);

        ezxml_free(val_xml);
    }

    // close up
    int result = fputs("</status>\n", fnew);
    fclose(fnew);

    if (result != EOF) // no error?
    {
        // we use rename-overwrite to ensure
        // atomic write
        rename(DATABASE ".new", DATABASE);
    }
    else
        perror("Write to database failed");
    
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

    puts("Measurement test - getset: OK");
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

    puts("Measurement test - persist: OK");
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

    puts("Measurement test - failpersist: OK");
}


static void test_getall()
{
    // fresh start
    unlink(DATABASE);
    store = NULL;

    measurement_set("wok", "baz");
    measurement_set("wor", "baz");
    measurement_set("bop", "baz");

    int l;
    const char **res = measurement_getkeys(&l);
    measurement_write();

    assert(l==3);
    assert(strcmp(res[0], "bop") == 0);
    assert(strcmp(res[1], "wok") == 0);
    assert(strcmp(res[2], "wor") == 0);

    
    puts("Measurement test - getall: OK");
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

