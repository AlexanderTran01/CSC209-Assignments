#!/bin/bash

# test to see whether the output of test_wordlist matches the expected output

# $# is the number of command line arguments 
if [ $# -ne 1 ]
then 
    echo "Usage: ./run_word_tests.sh inputfile"
    exit 1
fi

# $1 is the first command line argument and $2 is the second
./test_wordlist $1 > tmp.out
sort $1 > comp.out
sort tmp.out | diff comp.out -
if [ $? -eq 0 ]
then
    echo "Test passed"
    rm tmp.out comp.out
else 
    echo "Test failed: check tmp.out for output"
    rm comp.out
fi
