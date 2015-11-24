/*
 * Interface for a key value store
 * used to store measurements
 *
 */


#define DATABASE "measure-host.xml"


// Get/set the current measurement value
void         measurement_set(const char *key, const char *value);
const char * measurement_get(const char *key);

// Get an array of keys
// The count is passed back in count
const char ** measurement_getkeys(int *count);

// Write current state to disk
void measurement_write();

