#!/bin/bash

# Some end-to-end tests

S=localhost
P=6423

# start server
rm ./measure-host.xml
./measure-host &

# send some updates
echo "<update><k1>1.0</k1><k2>1.5</k2></update>" | nc $S $P

# two in one request
echo "<update><k2>1.4</k2><k3>enabled</k3></update><update>\
        <another>myval</another></update>" | nc $S $P

RES=`echo "<retrieve />" | nc $S $P`

#check some values
[[ "$RES" =~ "<k2>1.4</k2>" ]] || echo "FAIL: k2" $RES
[[ "$RES" =~ "<k3>enabled</k3>" ]] || echo "FAIL: k3" $RES
[[ "$RES" =~ "<another>myval</another>" ]] || echo "FAIL: another" $RES

# retrieve 1
RES=`echo "<retrieve><key>k1</key></retrieve>" | nc $S $P`
[[ "$RES" =~ "<k1>1.0</k1>" ]] || echo "FAIL: k1" $RES

#restart
kill $!
./measure-host &

# check persistence
RES=`echo "<retrieve><key>k1</key></retrieve>" | nc $S $P`
[[ "$RES" =~ "<k1>1.0</k1>" ]] || echo "FAIL: k1" $RES

kill $!

echo Done
