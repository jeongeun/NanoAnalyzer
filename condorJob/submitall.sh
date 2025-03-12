#!/bin/bash

# How to use
if [ "$#" -ne 1 ]; then
    echo "Usage: ./run_condor.sh <directory_path>"
    echo "Example: ./run_condor.sh ../samples/"
    exit 1
fi

# Get the directory path
INPUT_PATH="$1"
if [[ ! "$INPUT_PATH" = /* ]]; then
    # If relative path, convert to absolute
    INPUT_PATH="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
fi

# Check if the directory really exists
if [ ! -d "$INPUT_PATH" ]; then
    echo "[Error] Directory '$INPUT_PATH' cannot be found"
    exit 1
fi

echo "[INFO] Processing directory: $INPUT_PATH"

declare -a CORRS=("Base" "PU" "Rocco" "ID" "ISO" "All")

# Find sub directory and submit jdl
for corr in "${CORRS[@]}"; do
    for dir in "$INPUT_PATH"/*; do
        if [ -d "$dir" ]; then
            process_name=$(basename "$dir")
            echo "[INFO] Processing subdirectory: $dir"

            # Check for the Condor submission jdl script
            job_jdl_path="$dir//${corr}/job_${process_name}.jdl"

            if [ -f "$job_jdl_path" ]; then
                echo "[INFO] Found submission file: $job_jdl_path"
                cd "$dir//${corr}/" > /dev/null || { echo "[ERROR] Failed to change directory to $dir"; exit 1; }

                if condor_submit "job_${process_name}.jdl"; echo "condor_submit "job_${process_name}.jdl""; then
                    echo "[GOOD] Successfully submitted: job_${process_name}.jdl from $dir"
                else
                    echo "[ERROR] Failed to submit: job_${process_name}.jdl from $dir"
                fi
                cd - > /dev/null
            else
                echo "[WARNING] No submission file found: $job_jdl_path"
            fi
        fi
    done
done

echo "[INFO] Completed processing all subdirectories in $INPUT_PATH"
