#!/bin/bash

if [ $# -eq 1 ];
then
    # Your code here. (1/4)
    cat $1 | grep -E "WARN | ERROR"  > bug.txt

else
    case $2 in
    "--latest")
        # Your code here. (2/4)
	tail -5 $1	
    ;;
    "--find")
        # Your code here. (3/4)
	cat $1 | grep -E "$3" > $3.txt
    ;;
    "--diff")
        # Your code here. (4/4)
	diff -q $1 $3
    ;;
    esac
fi
