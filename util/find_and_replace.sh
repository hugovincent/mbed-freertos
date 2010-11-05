#!/bin/bash
# Helper script for find-and-replace on whole project
# Hugo Vincent, 5 November 2010

if [ $# -ne 2 ] ; then
	echo "Usage: find_and_replace search-term replace-with"
	echo "    Make sure you use the correct escaping for the regexes."
	exit
fi

if [ -e .build_tmp ] ; then
	make clean
fi

TMP=tmpfile_$$
for i in `find .` ; do
	FOUND=`grep $1 $i` 
	if [ "$FOUND" ] ; then
		echo "Replacing in $i"
		sed -e "s/$1/$2/g" < $i > $TMP
		mv $TMP $i
	fi
done
