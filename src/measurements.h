

#define DATABASE "measure-host.xml"



// Get/set the current measurement value
void         measurement_set(const char *key, const char *value);
const char * measurement_get(const char *key);

// Get all keys as a \0\0-terminated list of strings
// Caller should free()
char * measurement_getkeys();

// Write current state to disk
void measurement_write();

