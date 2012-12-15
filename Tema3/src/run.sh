#!/bin/bash

BIN=$1
TEST_FOLDER=$2
OUT_FILE=$3

if [ $# -ne 3 ]; then
    echo "Error: Usage $0 BIN TEST_FOLDER OUT_FILE"
    exit
fi

echo "Testing Julia..."

for i in $(seq 1 6)
do
    echo "Testing julia$i.in..."
    for j in 1 2 4 8 16
    do
        echo "$j processes"
        $TEST_FOLDER/script.sh $TEST_FOLDER/$BIN $TEST_FOLDER/test/julia$i.in $TEST_DIR$OUT_FILE $j 2>&1
        $TEST_FOLDER/test/imgdiff $TEST_FOLDER/test/julia$i.pgm $TEST_DIR$OUT_FILE 2>&1
    done
done

echo "Testing Mandelbrot..."

for i in $(seq 1 4)
do
    echo "Testing mandelbrot$i.in..."
    for j in 1 2 4 8 16
    do
        echo "$j processes"
        $TEST_FOLDER/script.sh $TEST_FOLDER/$BIN $TEST_FOLDER/test/mandelbrot$i.in $TEST_DIR$OUT_FILE $j 2>&1
        $TEST_FOLDER/test/imgdiff $TEST_FOLDER/test/mandelbrot$i.pgm $TEST_DIR$OUT_FILE 2>&1
    done
done
