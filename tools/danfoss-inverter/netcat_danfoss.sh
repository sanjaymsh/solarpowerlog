#!/bin/bash
if [[ $# -ne "1" ]]
then
	echo "Usage $0 <portnum>"
	exit 1
fi

if [[ ! -e ./netcat_danfoss_parse.sh ]]
then
	echo "./netcat_danfoss_parse.sh not found. cd into simulator dir"
	exit 1
fi

echo "The netcat script will listen on port $1. Press Ctrl+C to abort (maybe twice)"

# restart automatically if the solarpowerlog instance closed the connetion

while (/bin/sleep 5)
do
nc -l -p $1 -e ./netcat_danfoss_parse.sh
done
