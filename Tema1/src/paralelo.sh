#!/bin/bash

ANI=$1
FISIN=$2
FISOUT=$3
NUM_THREADS=$4

export OMP_SCHEDULE="dynamic"
if [ $# -eq 4 ]; then
    export OMP_NUM_THREADS=$4
fi

time ./paralelo $ANI $FISIN $FISOUT
