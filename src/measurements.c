/*
* measurements.c
*
* Maintains the set of current measurements
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>


#include "measurements.h"

#define DATABASE "measure-host.xml"

static int initialized = 0;
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


#ifdef TEST

void measurement_tests()
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

int main(void) {

   measurement_tests();


   return 0;
}

#endif

