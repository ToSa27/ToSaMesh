#!/bin/bash

case "$1" in
    -d|--daemon)
        $0 < /dev/null &> /dev/null & disown
        exit 0
        ;;
    *)
        ;;
esac

sudo killall ToSaMeshMaster
sudo killall node
sudo service mongodb stop
sudo rm /var/lib/mongodb/mongod.lock
rm -rf ../log
mkdir -p ../log
sudo service mongodb start
cd out
nohup sudo ./ToSaMeshMaster >> ../../log/ToSaMeshMaster.log 2>&1 &
cd ../../web
nohup node ToSaMeshWeb.js >> ../log/ToSaMeshWeb.log 2>&1 &

