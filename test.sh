#!/bin/bash

make clean
rm -rf ~/.track
make debug
./track
echo
echo 'select * from file; select * from file_version;' | sqlite3 ~/.track/db.sql
echo
find ~/.track
