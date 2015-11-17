# Design


## Assumptions

* I will assume the number of keys is limited and hence not optimize key-lookup.
* I will assume clients can misbehave, and keep connections open.

## Setup

The key/values will be kept in memory in the ezxml struct which seems to be sufficiently 
efficient, and easy to use for (de)serialization to clients and disk.

The application uses three modules with a simple linear dependency

    main.c           provides a select() loop to process parallel connections
      |
      |  request_process()
      V
    request.c        parses and handles a request from a client
      |
      |  measurements_set() / measurements_get()
      V
    measurements.c   maintains current measurements and (de)serializes them to disk



