/*
* request.c
*
* parses and processes individual requests
*/

#include <assert.h>
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <ctype.h> 

#include "measurements.h"

#include "request.h"



// processes the request passed
// returns NULL if the request is invalid
// or a response otherwise
// caller should free() the returned string
char *request_process(const char *request, int len)
{
   if (len == 0)
      len = strlen(request);

   char *result = malloc(1);
   return result;
}

// returns the length of the first request 
// passed in input or -1 if it contains 
// no full request
// searches for specific closing tags
int request_findend(const char *input)
{
   int found_end = -1, i, n;

   // We search for a (self) closing tag named one of these
   static const char *end_markers[] = { "update", "retrieve" };

   for(i = 0; i < sizeof(end_markers)/sizeof(char*); i ++)
   {
      int len = strlen(end_markers[i]);
      char *p = input;

      // loop though matches of this tag
      for(;;) 
      {
         p = strstr(p+1, end_markers[i]);
         if (p == NULL)
            break;

         n = (int)(p - input); // found position
         printf("Found tag at %d\n", n);

         if (n < 1 || (n > found_end && found_end!=-1))
            continue; // we found an earlier end

         // this is an end-marker if 
         // 1. preceded by </ end succeeded by WS + >
         if (n >= 2 && input[n -2] == '<' && input[n-1] == '/')
         {
            int q = n + len;
            while (isspace(input[q]))
               q++;
            if (input[q] == '>')
            {
               found_end = q+1;
               break;
            }
         }
         // 2. preceded by < end succeeded by WS + /> 
         else if (input[n-1] == '<')
         {
            int q = n + len;
            while(isspace(input[q]))
               q++;
            if (input[q] == '/' && input[q+1] == '>')
            {
               found_end = q + 2;
               break;
            }
         }
      }
   }

   printf("Found end %d in %s\n", found_end, input);

   return found_end;

}


#ifdef TEST


// Mock measurement functions
static int count_set = 0;
void measurement_set(const char *key, const char *value)
{
   count_set++;
}

const char * measurement_get(const char *key)
{
   return "wok";
}

void test_findend()
{
   // findend should take one request
   assert( request_findend("<update>x</update><update></update>") 
   == strlen("<update>x</update>")
);

// handle self-closing tags
assert( request_findend("<update />") == strlen("<update />"));

// without space
assert( request_findend("<retrieve/>") == strlen("<retrieve/>"));

// partial request
assert( request_findend("<update><mykey></mykey>") == -1); 
}


void test_update()
{
   count_set = 0;

   char *res = request_process(
      "<update>"
      "<mykey>myval</mykey>"
      "<myk1>1</myk1>"
      "</update>", 0
   );
   assert(*res == 0); // no response
   assert(count_set == 2); // 2 updates
   free(res);

   // test invalid
   request_process(
      "<update>"
      "<mykey>myval</my"
      "<myk1>1</myk1>"
      "</update>", 0
   );
   assert(res == NULL); // no response
   assert(count_set == 2); // still 2 updates

   // test empty update
   request_process( "<update />", 0);

   assert(*res == '\0'); // no response
   assert(count_set == 2); // still 2 updates
}

void test_receive()
{
   char *res = request_process(
      "<receive>"
      "  <key>key1</key>"
      "  <key>key2</key>"
      "</receive>", 0
   );

   // should be nicely aligned
   assert(strcmp(res, 
      "<status>\n"
      "  <key1>wok</key1>\n"
      "  <key2>wok</key2>\n"
      "</status>\n") == 0
   );

}

int main(void) {

   test_findend();
   test_update();
   //test_receive();
   puts("ALL OK");
   return 0;
}

#endif

