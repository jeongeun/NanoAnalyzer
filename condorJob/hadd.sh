#!/bin/bash

# How to use
if [ "$#" -ne 1 ]; then
    echo "Usage: ./hadd.sh <directory_path>"
    echo "Example: ./hadd.sh ./condorOut/era/"
    exit 1
fi

# Get the path of the input directory
INPUT_PATH="$1"
if [[ ! "$INPUT_PATH" = /* ]]; then
    INPUT_PATH="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
    #echo "INPUT_PATH = $INPUT_PATH"
fi

# Check if the directory exists
if [ ! -d "$INPUT_PATH" ]; then
    echo "[ERROR] Directory '$INPUT_PATH' does not exist"
    exit 1
fi

echo "[INFO] Processing directory: $INPUT_PATH"

# List of Correction types
corrs=("Base", "PU", "Rocco", "ID", "ISO", "All")

for process_dir in "$INPUT_PATH"/*/; do
    process_name=$(basename "$process_dir")
    echo "[INFO] Processing process: $process_name"

    # Loop over correction type
    for corr in "${corrs[@]}"; do
        corr_dir="${process_dir}${corr}/"
        echo "[INFO] Found correction type: $corr"

        # Check if Correction Type folder exists
        if [ -d "$corr_dir" ]; then
            echo "[INFO] Found correction type: $corr"

            # Check if there are root files to merge
            root_files=(${corr_dir}${process_name}_*.root)

            if [ -e "${root_files[0]}" ]; then
                # Perform hadd merge
                output_file="${INPUT_PATH}/htot_${process_name}_${corr}.root"
                echo "[INFO] Merging files in: $corr_dir"
                echo "hadd -f "$output_file" "${root_files[@]}""
                hadd -f "$output_file" "${root_files[@]}"
                echo "[SUCCESS] Created: $output_file"
            else
                echo "[WARNING] No root files found in: $corr_dir"
            fi
        else
            echo "[WARNING] Correction folder '$corr' does not exist for $process_name"
        fi
    done
done

echo "[INFO] All merging operations completed in $INPUT_PATH"


## Function to merge histo roots
#merge_files(){
#    local pattern="$1"
#    local output="$INPUT_PATH/htot_${pattern}.root"
#    local input_files=("$INPUT_PATH/${pattern}"/*.root)
#
#    # check the file exist
#    if [ -e "${input_files[0]}" ]; then
#        echo "[INFO] Merging $pattern samples..."
#        echo "hadd "$output" "${input_files[@]}"" || echo "[ERROR] Failed to merge $pattern"
#        #hadd "$output" "${input_files[@]}" || echo "[ERROR] Failed to merge $pattern"
#    else
#        echo "[WARNING] No files found for pattern '$pattern'"
#    fi
#}
#
#
#for process_dir in "$INPUT_PATH"/*/; do
#    process_name=$(basename "$process_dir")
#    merge_files "$process_name"
#done
#
#declare -a special_samples=(
#"DYto2L-2Jets_MLL-50"
#"SingleMuon_Run2022C"
#"WtoLNu-2Jets"
#)
#
#for sample in "${special_samples[@]}"; do
#    merge_files "$sample"
#done
#
#echo "[INFO] All merging operations completed in $INPUT_PATH"
#
#for process_dir in "$INPUT_PATH"/*; do
#    if [ -d "$process_dir" ]; then
#        process_name=$(basename "$process_dir")
#        echo "[INFO] Merging files for: $process_name"
#        hadd "$INPUT_PATH/hist_$process_name.root" "$process_dir"/*.root
#    fi
#done
#
## Special merging for specific samples
#echo "Performing special merging for specific samples..."
#
## Function to safely merge files
#merge_files() {
#    local pattern="$1"
#    local output="$2"
#    if ls "$INPUT_PATH"/hist_"$pattern"_*.root 1> /dev/null 2>&1; then
#        echo "Merging $pattern samples..."
#        hadd "$INPUT_PATH/$output" "$INPUT_PATH"/hist_"$pattern"_*.root
#    else
#        echo "Warning: No files found matching pattern $pattern"
#    fi
#}
#
## Merge specific sample groups
#merge_files "DYJetsToMuMu_M-10to50" "hist_DYJetsToMuMu_M-10to50.root"
#merge_files "WJetsToLNu_HT-100To200" "hist_WJetsToLNu_HT-100To200.root"
#merge_files "WJetsToLNu_HT-200To400" "hist_WJetsToLNu_HT-200To400.root"
#merge_files "WJetsToLNu_HT-400To600" "hist_WJetsToLNu_HT-400To600.root"
#merge_files "WJetsToLNu_HT600To800" "hist_WJetsToLNu_HT600To800.root"
#merge_files "WJetsToLNu_HT800To1200" "hist_WJetsToLNu_HT800To1200.root"
#merge_files "WJetsToLNu_HT1200To2500" "hist_WJetsToLNu_HT1200To2500.root"
#merge_files "WJetsToLNu_HT2500ToInf" "hist_WJetsToLNu_HT2500ToInf.root"
#merge_files "WToMuNu_M-200" "hist_WToMuNu_M-200.root"
#
#echo "All merging operations completed in $INPUT_PATH"
#
