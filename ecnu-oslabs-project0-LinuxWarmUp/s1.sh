#!/bin/bash
#s1.sh

#create a new directory containing s1.sh
mkdir -p $1
cp s1.sh $1

#create two new files containing my name and student number
echo "weiyang" > name.txt
echo "10142130214" > stno.txt

#copy the two files to the directory ./foo
cp name.txt $1
cp stno.txt $1
