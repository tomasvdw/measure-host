/*
* measurements.c
*
* Maintains the set of current measurements
 */

#include <stdlib.h>

#include <unistd.h>
#include <assert.h>


#include "measurements.h"


static void init()
{

}


void measurement_set(const char *key, const char *value)
{
   init();
}

const char * measurement_get(const char *key)
{
   return "";
}

void measurement_write()
{
}


#ifdef TEST

static void test_getset()
{
   // fresh start
   unlink(DATABASE);

   measurement_set("wok", "bar");
   measurement_set("foo", "3.0");
   measurement_set("foo", "2.0");

   assert(strcmp(measurement_get("wok"), "bar") == 0);
   assert(strcmp(measurement_get("foo"), "2.0") == 0);

   puts("Measurement tests OK");
}

static void test_persist()
{
   // fresh start
   unlink(DATABASE);

   measurement_set("wok", "baz");
   measurement_write();

   // force reload
   init();


   // should get data from disk
   assert(strcmp(measurement_get("wok"), "baz") == 0);
}


int main(void) {

   test_getset();
   test_persist();


   return 0;
}

#endif

