#!/bin/bash

INT=${1:-./kissfuck}

# runs time benchmark
echo "This is benchmake for $INT virtual machine! :p"
echo " You can pass any other vm as first argument! "
echo "    time utility required to be installed!    "
echo "----------------------------------------------"

for fl in ./tests/*; do
	echo "-------------------------------------------"
	echo "running for a file $fl"
	echo "                                           "
	sudo chrt -f 99 time -f "\n\nelapsed=%E cpu=%P returns=%x" $INT $fl
done
