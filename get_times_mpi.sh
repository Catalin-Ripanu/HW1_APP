#!/bin/bash

tests=("./data/test1.in" "./data/test2.in" "./data/test3.in")
nr_of_processes=("2" "4" "8" "16" "32" "64" "128" "256" "512")

for i in ${nr_of_processes[@]};
do
	echo "********************Start of NrProc = $i********************"
	for j in ${tests[@]};
do
	echo "************************************************************"
	cmd="time ./mpi_$1_cluster.sh $i $j"
	eval $cmd
	echo "************************************************************"
done
	echo "********************End of NrProc = $i**********************"
done
