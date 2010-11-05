#!/bin/bash
# Helper script for find-and-replace on whole project
# Hugo Vincent, 5 November 2010

if [ $# -ne 2 ] ; then
	echo "Usage: find_and_replace search-term replace-with"
	echo "    Make sure you use the correct escaping for the regexes."
	exit
fi

TMP=tmpfile_$$

find . -name "*.c"      > $TMP
find . -name "*.cpp"   >> $TMP
find . -name "*.h"     >> $TMP
find . -name "*.S"     >> $TMP
find . -name "*.py"    >> $TMP
find . -name "*.sh"    >> $TMP
find . -name "*.mk"    >> $TMP
find . -name "*.ld"    >> $TMP
find . -name "*.md"    >> $TMP
find . -name "*.txt"   >> $TMP
find . -name "*.vim"   >> $TMP
find . -name "*.css"   >> $TMP
find . -name "*.html"  >> $TMP
find . -name "*.shtml" >> $TMP

FILES=`grep -v .git $TMP`
rm $TMP

for i in $FILES ; do
	FOUND=`grep $1 $i` 
	if [ "$FOUND" ] ; then
		echo "Replacing in $i"
		sed -e "s/$1/$2/g" < $i > $TMP
		mv $TMP $i
	fi
done
