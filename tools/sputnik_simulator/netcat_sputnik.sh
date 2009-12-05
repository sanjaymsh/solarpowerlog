#!/bin/bash

function parse()
{
    read -n 10 >/dev/null  
    echo {01;FB;9B|64:PAC=AA;KHR=12D8;PIN=11F8;KT0=842;DDY=5;KYR=687;KMT=1;KDY=1;KT0=842;UDC=890;IDC=32;IL1=24;UL1=920;TKK=1A;TMI=26;THR=C;SYS=4E24,0;TNF=1386|256B}
    read -t 1 >/dev/null
}

declare -fx parse

nc -l -p $1 -e ./netcat_sputnik_parse.sh
#nc -l -p $1 -e parse









#69931 [0x40625dd0] TRACE inverter.Inverter_2 null - Sending: {FB;01;5E|64:PAC;KHR;PIN;KT0;DDY;KYR;KMT;KDY;KT0;RPL;UDC;IDC;IL1;UL1;TKK;TMI;THR;SYS;TNF|185F}

#70247 [0x40625dd0] TRACE inverter.Inverter_2 null - Received :{01;FB;9B|64:PAC=AA;KHR=12D8;PIN=11F8;KT0=842;DDY=5;KYR=687;KMT=1;KDY=1;KT0=842;UDC=890;IDC=32;IL1=24;UL1=920;TKK=1A;TMI=26;THR=C;SYS=4E24,0;TNF=1386|256B}len: 155
