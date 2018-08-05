#!/bin/bash

# linux 
sudo apt-get install gtk+-3.0
sudo apt-get install tree


# windows
mydir=$PWD
cd $mydir/libs/windows/gtk+-bundle_3.10.4-20131202_win64/
find -name '*.pc' | while read pc; do sed -e "s@^prefix=.*@prefix=$PWD@" -i "$pc"; done
cd $mydir





