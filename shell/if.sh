#!/bin/sh
IP_A=  dd
IP_B=
if [ -n "$IP_A" ] || [  -n "$IP_B" ];then
    echo "no IP_A"
else
    echo "xxxxxxxxxxxxx $IP_A"
fi
