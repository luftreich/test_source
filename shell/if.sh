#!/bin/sh
IP_A=dd
IP_B=
if [ -n "$IP_A" ] || [  -n "$IP_B" ];then
    echo "no IP_A"
else
    echo "xxxxxxxxxxxxx $IP_A"
fi


if [ ! -d '/binls' ]; then
echo "bin/ls  is NOT a  dir"
else
echo "ls is a dir"
fi

if [ -d '/home' ]; then
echo "home is a dir"
else
echo "home is not a dir"
fi
