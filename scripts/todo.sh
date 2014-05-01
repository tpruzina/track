#!/bin/bash
find . -name '*.[c,h]' | xargs grep -iE '.*\/\/.*todo'
