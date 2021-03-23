#!/bin/bash
result=`ps -ef |grep main |grep -v grep |awk '{print $2}'`
#echo $result
if [ -z $result ]
then
	echo "no such a program"
elif [ -n $result ]
then
	kill -9 $result
fi
