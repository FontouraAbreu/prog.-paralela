#!/bin/bash

# Set paths
TEST_DIR="./testes"
OUTPUT_DIR="./output"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Loop through all *_A.in files
for fileA in "$TEST_DIR"/*_A.in; do
    # Extract base name (e.g., 50000 from 50000_A.in)
    base=$(basename "$fileA" _A.in)
    fileB="$TEST_DIR/${base}_B.in"

    # Check if matching B file exists
    if [ -f "$fileB" ]; then
        echo "Running test for size $base..."

        # Run sequential version
        ./seq "$fileA" "$fileB" > "$OUTPUT_DIR/${base}_seq.out"

        # Run parallel version
        ./par "$fileA" "$fileB" > "$OUTPUT_DIR/${base}_par.out"
    else
        echo "Missing file: ${fileB}"
    fi
done

echo "All tests completed. Results saved in $OUTPUT_DIR"
