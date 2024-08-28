#!/bin/bash

module load mpi/openmpi-x86_64
mpirun -np $1 --oversubscribe --mca btl_tcp_if_include eth0 ./kmeans_mpi $2
