#!/bin/bash
mkdir mydir
chmod a+rwx mydir
touch myfile
echo 2023 > myfile
mv moveme ./mydir
cp copyme ./mydir
cd mydir/
mv copyme copied
cd ..
cat readme
gcc bad.c 2> err.txt
mkdir gen
cd gen/
n=10
if [ $# -ne 0 ]; then n=$1; fi
i=1
while [ $i -le $n ] 
do
	touch $i.txt
	let i=i+1
done
cd ..
