

#define DATABASE "measure-host.xml"



// Get/set the current measurement value
void         measurement_set(const char *key, const char *value);
const char * measurement_get(const char *key);


// Write current state to disk
void measurement_write();

