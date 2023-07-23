#!/bin/sh
# assignment 1 finder.sh creation
# Author: Abdullah Alabd
# GitUserName: alabd0

filesdir=$1
searchstr=$2
# number of Y lines that contain the search pattern: grep -rs "$filesdir" -e "$searchstr" | wc -l
# number of X files in that directory and other directorties: find "$filesdir" -type f | wc -l
# -d "$DIR" dirwctiry exist or nor

if [ $# -eq 0 ] || [ -z $filesdir ] || [ -z $searchstr ] || [ ! -d "$filesdir" ] #if the specific arguments isn't exist return 1
then
echo Error In The Prameters
exit 1
else
Y=$( grep -rs "$filesdir" -e "$searchstr" | wc -l )
X=$( find "$filesdir" -type f | wc -l )
echo "The number of files are $X and the number of matching lines are $Y"
exit 0
fi

