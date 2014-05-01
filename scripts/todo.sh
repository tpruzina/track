#!/bin/bash
find . -name '*.[c,h]' | xargs grep -n -iE '.*\/\/.*todo'
