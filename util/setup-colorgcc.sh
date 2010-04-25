#!/bin/bash

# Find where arm-eabi-* is installed FIXME
TOOLDIR=`dirname \`which arm-eabi-gcc\``

# Install wrapper symlinks
for i in `ls $TOOLDIR | grep arm-eabi-`; do 
	ln -s colorgcc $i
done

