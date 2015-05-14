#!/bin/sh
while read a b
do
echo $a
echo $b
done < $1
