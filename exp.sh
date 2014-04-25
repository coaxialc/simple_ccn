#! /bin/bash

#${1} is the topology file path
#${2} is the path of output directory
#${3} is the seed for the random object (usually 80)
seed=$3
group_size=1 #the group size
#${4} is the number of interest requests to be transmitted

#loop through different group sizes
while true; do 
	
	  ./waf --run "app2 $1 $2 $seed $group_size $4"

	#If the group size we asked to be used could not be used because there were not
        #as many nodes with degree 1 then c++ returned 1 and its time
        #to end the application. 
	if [ $? = 1 ]; then
		break
	fi

	group_size=$[group_size+1]
        seed=$[seed+1]

done	

#Parse results
#python parse2.py
