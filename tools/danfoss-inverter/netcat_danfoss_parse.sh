#!/bin/bash

while (/bin/true)
do
    read -t 1 1>&2  
    echo 7EFF03120300020A01C80D40010247000E27072AE77E | xxd -r -p || exit 0
    read -t 1 1>&2  
    touch /tmp/stampme
done 
