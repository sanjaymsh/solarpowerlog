#!/bin/bash

while (/bin/true)
do
    read -n 10 >/dev/null  
    echo -n '{01;FB;9B|64:PAC=AA;KHR=12D8;PIN=11F8;KT0=842;DDY=5;KYR=687;KMT=1;KDY=1;KT0=842;UDC=890;IDC=32;IL1=24;UL1=920;TKK=1A;TMI=26;THR=C;SYS=4E24,0;TNF=1386|256B}' || exit 1
    read -t 1 >/dev/null
    touch /tmp/stampme
done 
