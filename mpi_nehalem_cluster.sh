#!/bin/bash

module load mpi/openmpi-x86_64
mpirun -np $1 --oversubscribe ./kmeans_mpi $2
