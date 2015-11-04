#!/bin/sh
test_case ()
{
    case "$1" in
        Linux)
        echo "Linux ";;
        Macos)
        echo "Macos";;
        *)
        echo "unknown";;
    esac;
}

test_case $1
