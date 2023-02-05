#!/bin/bash

INT=${1:-./kissfuck}


for fl in ./tests/*; do
	echo -e "\n-------------------------------------------"
	echo "running for a file $fl"
	echo "                                           "

	$INT $fl || exit -1
done

echo -e "\nAll tests passed!"
exit 0
