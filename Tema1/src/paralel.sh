#!/bin/bash

ANI=$1
FISIN=$2
FISOUT=$3

export OMP_SCHEDULE="dynamic"
export OMP_NUM_THREADS=1

time ./paralel $ANI $FISIN $FISOUT
