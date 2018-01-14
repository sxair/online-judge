#! /bin/bash 
pid=`cat /home/judge/etc/judge.pid`
if ps -aux | grep $pid | grep -q judge;then
	kill -SIGINT $pid
	echo "shutdown success"
else
	echo "judge not exist"
fi