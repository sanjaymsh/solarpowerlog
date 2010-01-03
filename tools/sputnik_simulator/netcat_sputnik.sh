#!/bin/bash

if [[ $# -ne "1" ]]
then
	echo "Usage $0 <portnum>"
	exit 1
fi

if [[ ! -e ./netcat_sputnik_parse.sh ]] 
then
	echo "./netcat_sputnik_parse.sh not found. cd into simulator dir"
	exit 1
fi

echo "The netcat script will listen on port $1. Press Ctrl+C to abort (maybe twice)"


# restart automatically if the solarpowerlog instance closed the connetion
while (/bin/sleep 5)
do
nc -l -p $1 -e ./netcat_sputnik_parse.sh
done


#69931 [0x40625dd0] TRACE inverter.Inverter_2 null - Sending: {FB;01;5E|64:PAC;KHR;PIN;KT0;DDY;KYR;KMT;KDY;KT0;RPL;UDC;IDC;IL1;UL1;TKK;TMI;THR;SYS;TNF|185F}

#70247 [0x40625dd0] TRACE inverter.Inverter_2 null - Received :{01;FB;9B|64:PAC=AA;KHR=12D8;PIN=11F8;KT0=842;DDY=5;KYR=687;KMT=1;KDY=1;KT0=842;UDC=890;IDC=32;IL1=24;UL1=920;TKK=1A;TMI=26;THR=C;SYS=4E24,0;TNF=1386|256B}len: 155
