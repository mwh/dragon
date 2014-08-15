#!/bin/sh
# This toy example uses wget and the --target mode to act
# as a drag-and-drop download tool.

../dragon --target | while read url
do
    wget "$url"
done
