#!/bin/bash

usage()
{
    echo "USAGE: $0 [«number-of-runs»] (default 1000)"
}

error()
{
    echo -e "\e[31;1m${1}\e[0m"
}

case $# in
    0) n=1000;;
    1) n=$1;;
    *) error "Wrong number of arguments"; 
       usage; 
       exit 1;;
esac

if ! [ $n -gt 0 ] 2>/dev/null; then
    error "Wrong argument value (\"$n\")."
    usage
    exit 1
fi

for i in $(seq 1 $n)
do
     echo -e "\n\e[34;1mRun n.º $i\e[0m"
     echo -e "stat\ny" | ./probMonGameOfRope
done

