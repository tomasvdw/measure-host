

# measure-host

This is a small service application to retrieve and send measurement
data. It will accept connections on TCP port 6423, and accepts requests
to update and retrieve data

## install

To build and run the unit tests use

   make test

To build the server use

   make

It is tested on Linux, but should work on any \*NIX os; 

Some end-to-end can be run using

   ./test-ee.sh

To start the server use

   ./measure-host

## usage

The server will accept incoming connections on 6243 and will understand 
requests to update current measurement values  in the format:

    <update>
       <myKey>myValue</myKey>
       <anotherKey>1</anotherKey>
    </update>

Current values can be requested by sending:

    <retrieve>
       <key>myKey</key>
       <key>anotherKey></key>
    </retrieve>

To request all current values, you can send

    <retrieve />

You can send multiple requests over the same connection. If any invalid
data is received, the server will close the connection silently.

