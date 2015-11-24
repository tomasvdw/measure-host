# Design



## Assumptions

* The number of keys is limited and hence not optimize key-lookup.
* Clients can misbehave, and keep connections open.
* Client will not be malicious

## Setup

The key/values will be kept in memory in the data structure used by the ezxml library. This seems to be sufficiently 
efficient, and easy to use for (de)serialization to clients and disk.

The persistence format is the same XML format as returned to the client

The application uses three modules with a simple linear dependency

    main.c           The tcp server; This uses a select() loop to process parallel connections
      |
      |  request_process() 
      V
    request.c        Request handling; parses and handles a request from a client
      |
      |  measurements_set() / measurements_get()
      V
    measurements.c   Storage; maintains current measurements and (de)serializes them to disk


## Parallel connections

The server uses select() to prevent read() and accept() operations from blocking; this way it can
handle multiple parallel connections.

The response are send using a blocking write(), as the responses are small, they will fit in the receive buffer
on the clients end, and hence only block during the send itself.


