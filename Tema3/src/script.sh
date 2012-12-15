#!/bin/bash

BIN=$1
FILEIN=$2
FILEOUT=$3
NUM_THREADS=$4

if [ $# -ne 4 ]; then
	echo "Error: Usage $0 BIN FILEIN FILEOUT NUM_THREADS"
	exit
fi

module load libraries/openmpi-1.6-gcc-4.4.6

time mpirun -n $NUM_THREADS $BIN $FILEIN $FILEOUT
