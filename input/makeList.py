#!/usr/bin/env python3
import os
import argparse
from pathlib import Path

SE_KNU="dcap://cluster142.knu.ac.kr/"

def get_folder(base_dir, max_depth=3):
    """
    Given a base directory, return the latest (most recent) timestamp subfolder.
    """
    folders = [d for d in Path(base_dir).glob("**/") if d.is_dir() and len(d.relative_to(base_dir).parts) <= max_depth]
    if not folders:
        return None

    return max(folders, key=lambda d: d.stat().st_mtime)


def get_files(base_dir):
    """
    Recursively searches for .root under the recent timestamped folder in the base_dir and returns their full paths.
    """
    tree_folder = get_folder(base_dir)
    if tree_folder:
        return [SE_KNU + str(file) for file in tree_folder.rglob("*.root")]
        #return [str(file) for file in tree_folder.rglob("*.root")]
    return []

def main():
    parser = argparse.ArgumentParser(
        description="Generate a list of .root files for each process in list_<era>.list"
    )
    parser.add_argument('-b', '--base_dir', default="/pnfs/knu.ac.kr/data/cms/store/user/jelee/crab_NanoAOD",
                        help='Base directory to search for .root files.')
    parser.add_argument('-l', '--list_dir', default='.', help='Directory containing list_<era>.txt files.')
    parser.add_argument('-o', '--output_dir', default='.', help='Output directory for result files.')
    parser.add_argument("-e", "--era", default="2022preEE",
                        help='Specify a single era (2016preVFP, 2016postVFP, 2017, 2018, 2022preEE, 2022postEE).')

    args = parser.parse_args()
    
    #Determine the eras to process
    era = args.era
    #eras = [args.era] if args.era else ["2016preVFP", "2016postVFP", "2017", "2018", "2022preEE", "2022postEE"]

    list_file_path = Path(args.list_dir) / f"list_{era}.txt"
    if not list_file_path.exists():
        print(f"List file {list_file_path} does not exist. Skipping era {era}.")
        return

    with list_file_path.open('r') as lf:
        processes = [line.strip() for line in lf if line.strip()]

    # Create the output directory for this era
    era_output_dir = Path(args.output_dir) / era
    era_output_dir.mkdir(parents=True, exist_ok=True)

    for process in processes:
        process_dir = Path(args.base_dir) / era / process
        if not process_dir.is_dir():
            print(f"Directory {process_dir} does not exist. Skipping process '{process}' for era {era}.")
            continue

        # Get files only from the latest timestamped folder
        matching_files = get_files(process_dir)

        output_file = era_output_dir / f"{process}.list"
        print("@@ output_file", output_file, " ### process.list", process)

        # Write results to output file
        with output_file.open('w') as out_f:
            out_f.writelines(f"{mf}\n" for mf in matching_files)

        print(f"For era {era}, process '{process}': found {len(matching_files)} .root files. Written to {output_file}")

if __name__ == '__main__':
    main()
