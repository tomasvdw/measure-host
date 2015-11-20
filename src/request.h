

// returns the length of the first request 
// passed in input or -1 if it contains 
// no full request
int request_findend(const char *input);


// processes the request passed
// returns NULL if the request is invalid
// or a response otherwise
// caller should free() the returned string
char *request_process(char *request, int len);

