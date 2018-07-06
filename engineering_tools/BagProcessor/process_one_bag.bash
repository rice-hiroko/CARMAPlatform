#!/bin/bash
killall roscore || echo "roscore was not running."
killall rosmaster || echo "rosmaster was not running."
bagfile=$1
filter=$2
folder=$(basename "$bagfile" .bag);
command=""
file=topics.txt
while read -r line; do
    [[ "$line" =~ ^#.*$ ]] && continue
    command=$command"rostopic echo -p "${line}" & "
done < "$file"
command=${command%?}""
command=${command%?}""
command=${command%?}";"
duration=$((rosbag info $bagfile | grep duration) | awk -F'[()]' '{gsub(/[^0-9 ]/,"",$2); print $2}')
echo $duration
if [[ $duration == "" ]]
then
	echo "The bag file's duration was less than 60 seconds, so it was not processed"
else
	if [[ $duration -gt $filter ]]
	then
		sleep 2s
		mkdir $folder
		mv ./$1 ./$folder
		mv route.txt ./$folder
		cd $folder
		roscore &
		sleep 5s
		xterm -e "$command" &
		sleep 12s
		rosbag play -r 10 $bagfile
		sleep 5s
		killall rostopic
		if [[ -s output/nav_sat_fix.csv ]]
		then
    			echo "nav_sat_fix did not work"
    			folder=$(basename "$1" .bag)
    			cat route.txt | grep Downtrack: > position.csv;
    			mv position.csv $folder
		else
    			echo "nav_sat_fix worked!"
		fi
		killall roscore
		killall rosmaster
		mv ./$1 ../
		mv route.txt ../
	else
		echo "The bag file's duration was only $duration seconds, so it was not processed"
	fi
fi
