#!/bin/sh
# assignment 1 writer.sh creation
# Author: Abdullah Alabd
# GitUserName: alabd0

writefile=$1
writestr=$2
direname=$( dirname $writefile )

# -e "$writefile"  file exist or not

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