#!/bin/sh
# assignment 1 writer.sh creation
# Author: Abdullah Alabd
# GitUserName: alabd0

writefile=$1
writestr=$2
direname=$( dirname $writefile )

# number of Y lines that contain the search pattern: grep -rs "$filesdir" -e "$searchstr" | wc -l
# number of X files in that directory and other directorties: find "$filesdir" -type f | wc -l
# -d "$DIR" dirwctiry exist or nor

if [ $# -eq 0 ] || [ -z $writefile ] || [ -z $writestr ]; then #if the specific arguments isn't exist return 1
    echo Error In The Prameters File Won\'t be craeted
    exit 1
elif [ ! -e "$writefile" ]; then
    mkdir -m 775 -v -p $direname
    touch $writefile && chmod 775 $writefile
fi

if [ -e "$writefile" ]; then
    echo $writestr > $writefile
    
    echo File Created And Its Content: && cat $writefile
    exit 0
else
    echo File Won\'t be Created 
    exit 1
fi