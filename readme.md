

# measurement-host

This is a small service application to retrieve and send measurement
data. It will accept connections on TCP port 6423, which can either 
update existing keys using an update request:

    <update>
       <myKey>myValue</myKey>
       <anotherKey>1</anotherKey>
    </update>

or retrieve a set of values:

    <retrieve>
       <key>myKey</key>
       <key>anotherKey></key>
    </retrieve>

More detailed specification is available in an external document.


