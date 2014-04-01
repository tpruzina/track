#!/bin/bash

make clean
rm -rf .track
make debug
./track --add `find . -type f -name '*.[c,h]'`
echo
echo 'select * from file; select * from file_version; select * from snapshot; select * from snapshot_file;' | sqlite3 .track/db.sql
echo
find .track
