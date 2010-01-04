#!/bin/bash

echo "Connected" 1>&2

while (/bin/true)
do
    echo -n "." 1>&2
    read -n 10 >/dev/null 
    echo -n '{01;FB;AE|64:TYP=4E2A;SWV=F;BDN=66B;PAC=0;KHR=138E;PIN=11F8;KT0=863;DDY=4;KYR=1;KMT=1;KDY=1;KT0=863;UDC=821;IDC=4;IL1=0;UL1=905;TKK=18;TMI=35;THR=10;TNF=1385;SYS=4E22,0|2A78}'
    read -t 1 >/dev/null
    read -n 10 >/dev/null 
    echo -n '{01;FB;97|64:PAC=0;KHR=138E;PIN=11F8;KT0=863;DDY=4;KYR=1;KMT=1;KDY=1;KT0=863;UDC=80A;IDC=4;IL1=0;UL1=8F5;TKK=18;TMI=38;THR=10;TNF=1384;SYS=4E22,0|246E}' || exit 1
    read -t 1 >/dev/null
    touch /tmp/stampme
done 

