#!/usr/bin/env python3
import os
import sys
import argparse
import math
from pathlib import Path

def split_file(era, process_name, Nsplit, base_dir):
    """Splits the process text file into smaller chunks with Nsplit lines each."""
    
    # Construct paths
    input_file = Path(base_dir) / era / f"{process_name}.list"
    output_dir = Path(base_dir) / era / process_name

    # Ensure input file exists
    if not input_file.exists():
        print(f"Error: Input file {input_file} does not exist.")
        return

    # Read lines from input file
    with input_file.open("r") as f:
        lines = f.readlines()

    if not lines:
        print(f"Warning: Input file {input_file} is empty. Nothing to split. Skipping.")
        return

    # Create output directory if not exists
    output_dir.mkdir(parents=True, exist_ok=True)

    # Determine number of split files
    num_files = math.ceil(len(lines) / Nsplit)

    for i in range(num_files):
        output_file = output_dir / f"{process_name}_{i}.list"
        with output_file.open("w") as out_f:
            out_f.writelines(lines[i * Nsplit : (i + 1) * Nsplit])
        print(f"Written {len(lines[i * Nsplit : (i + 1) * Nsplit])} lines to {output_file}")

    # Delete original input file after splitting
    try:
        input_file.unlink()
        print(f"Deleted original file: {input_file}")
    except Exception as e:
        print(f"Error deleting file {input_file}: {e}")



def main():
    parser = argparse.ArgumentParser(description="Split process files into smaller chunks based on list_<era>.txt")
    parser.add_argument("-b", "--base_dir", type=Path, default=Path("./"), help="Base directory for file lists (default: ./)")
    parser.add_argument("-l", "--list_dir", type=Path, default=Path("."), help="Directory containing list_<era>.txt files (default: current directory)")
    parser.add_argument("-e", "--era", default="2022preEE", help="Specify a single era (e.g. 2016APV, 2016, 2017, 2018), or process all eras.", required=False)
    parser.add_argument("-n", "--Nsplit", type=int, default=20, help="Number of lines per split file (default: 20)")

    args = parser.parse_args()

    # If no specific era is provided, process all. 
    eras = [args.era] if args.era else ["2016preVFP", "2016postVFP", "2017", "2018", "2022preEE"]
    
    for era in eras:
        # Read the process names from list_<era>.txt located in the list_dir.
        list_file = args.list_dir / f"list_{era}.txt"

        if not list_file.exists():
            print(f"List file {list_file} does not exist. Skipping era {era}.")
            continue

        with list_file.open("r") as f:
            processes = [line.strip() for line in f if line.strip()]
        
        for process in processes:
            split_file(era, process, args.Nsplit, args.base_dir)

if __name__ == "__main__":
    main()
