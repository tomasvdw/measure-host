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

#include "ezxml.h"
#include "measurements.h"
#include "request.h"


// handles parsed update request
static void process_update(ezxml_t request)
{
    // set each child in the measurement store
    ezxml_t child = request->child;
    while(child != NULL)
    {
        // key = tagname, value=text content
        measurement_set(child->name, ezxml_txt(child));

        child = child->sibling;
    }
    measurement_write();
}

// Finds the value for the given key and appends 
// as xml to *result
// *result is realloced and *alloced is updated
static void append_result(const char *key, char **result, int *alloced)
{
    const char *val = measurement_get(key);

    // construct xml for this key/val
    ezxml_t val_xml = ezxml_new(key);
    ezxml_set_txt(val_xml, val);
    char *t = ezxml_toxml(val_xml);

    // make space in result
    *alloced += strlen(t) + 3; // include WS
    *result = realloc(*result, *alloced);

    // add line to result
    strcat(*result, "  ");
    strcat(*result, t);
    strcat(*result, "\n");

    free(t);
    ezxml_free(val_xml);
}


// handles parsed retrieve request;
// returns xml doc that must be freed
static char * process_retrieve(ezxml_t request)
{
    // initialize result-doc
    int alloced = strlen("<status>\n</status>\n")+1;
    char *result = malloc(alloced);
    strcpy(result, "<status>\n");

    // browse requested keys
    ezxml_t child = ezxml_child(request, "key");
    if (child == NULL)
    {
        // no children means retrieve all
        char *allkeys = measurement_getkeys();
        char *p = allkeys;
        while(*p) // end at \0\0
        {
            append_result(p, &result, &alloced);
            p += strlen(p) + 1;
        }
        free(allkeys);
    }
    else
    {
        while(child != NULL)
        {
            append_result(ezxml_txt(child), &result, &alloced);

            child = ezxml_next(child);
        }
    }
    strcat(result, "</status>\n");
    return result;
}


// processes the request passed
// returns NULL if the request is invalid
// or a response otherwise
// caller should free() the returned string
char *request_process(char *request, int len)
{

    // parse xml-request
    ezxml_t rq = ezxml_parse_str(request, len);
    if (rq == NULL || rq->name == NULL)
    {
        perror(ezxml_error(rq));
        return NULL;
    }

    // delegate
    if (strcmp(rq->name, "update") == 0)
    {
        process_update(rq);

        ezxml_free(rq);
        return strdup("");
    } 
    else if (strcmp(rq->name, "retrieve") == 0)
    {
        char *resp = process_retrieve(rq);

        ezxml_free(rq);
        return resp;
    }
    else
    {
        // unknown tag
        ezxml_free(rq);
        return NULL;
    }

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
        char *p = (char *)input;

        // loop though matches of this tag
        for(;;) 
        {
            p = strstr(p+1, end_markers[i]);
            if (p == NULL)
                break;

            n = (int)(p - input); // found position

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

    return found_end;
}


#ifdef TEST

// Mock measurement functions
static int count_set = 0;
void measurement_set(const char *key, const char *value)
{
    count_set++;
}

void measurement_write()
{
}

const char * measurement_get(const char *key)
{
    return "wok";
}

// mock getting all keys
char * measurement_getkeys()
{
    char *p = malloc(50);
    memcpy(p, "key1\0key2\0\0", 11);
    return p;
}

void test_findend()
{
    // findend should take one request
    assert( 
            request_findend("<update>x</update><update></update>") 
            == strlen("<update>x</update>")
          );

    // handle self-closing tags
    assert( request_findend("<update />") == strlen("<update />"));

    // without space
    assert( request_findend("<retrieve/>") == strlen("<retrieve/>"));

    // partial request
    assert( request_findend("<update><mykey></mykey>") == -1); 
}

// test with const char's
char * dotest_process(const char *p)
{
    char *m = strdup(p);
    char *res = request_process(m, strlen(m));
    free(m);
    return res;
}

void test_update()
{
    count_set = 0;

    char *res = dotest_process(
            "<update>"
            "<mykey>myval</mykey>"
            "<myk1>1</myk1>"
            "</update>"
            );
    assert(*res == 0); // no response
    assert(count_set == 2); // 2 updates
    free(res);

    // test invalid
    res = dotest_process(
            "<wok>"
            "<mykey>myval</mykey>"
            "<myk1>1</myk1>"
            "</wok>"
            );
    assert(res == NULL); // no response
    assert(count_set == 2); // still 2 updates

    // test empty update
    res = dotest_process( "<update />");

    assert(*res == '\0'); // no response
    assert(count_set == 2); // still 2 updates
}

void test_receive()
{
    char *res = dotest_process(
            "<retrieve>"
            "  <key>key1</key>"
            "  <key>key2</key>"
            "</retrieve>"
            );

    // should be nicely aligned
    assert(strcmp(res, 
                "<status>\n"
                "  <key1>wok</key1>\n"
                "  <key2>wok</key2>\n"
                "</status>\n") == 0
          );

}

void test_receive_all()
{
    char *res = dotest_process(
            "<retrieve />"
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
    test_receive();
    test_receive_all();
    puts("Request tests OK");
    return 0;
}

#endif

