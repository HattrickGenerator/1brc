#!/bin/bash

#Usage: Run test.sh for all tests or e.g. test.sh ../../test_cases measurements-short for only running a single test case

# Function to compare output files
compare_output() {
    if diff -w out_ref.txt "$1"; then
        echo "Output files are identical."
    else
        echo "Output files are different from $1 Exiting..."
        exit 1
    fi
}

# Check if an input argument is provided
if [[ -n "$1" ]]; then
    # Use provided test case name
    txt_file="../../test_cases/$1.txt"
    out_file="../../test_cases/$1.out"

    # Check if corresponding *.out file exists
    if [[ -f $out_file ]]; then
        # Copy *.txt to measurements.txt in the current directory
        cp "$txt_file" measurements.txt
        
        # Run ./analyze and redirect output to out_ref.txt
        ./1brc > out_ref.txt
        
        # Compare output files
        compare_output "$out_file"
    else
        echo "Corresponding .out file for $1 not found. Exiting..."
        exit 1
    fi
else
    # Run for all test cases
    for pair in ../../test_cases/*.txt; do
        txt_file="$pair"
        out_file="${pair%.txt}.out"

        # Check if corresponding *.out file exists
        if [[ -f $out_file ]]; then
            # Copy *.txt to measurements.txt in the current directory
            cp "$txt_file" measurements.txt
            
            # Run ./analyze and redirect output to out_ref.txt
            ./1brc > out_ref.txt
            
            # Compare output files
            compare_output "$out_file"
        fi
    done
fi
