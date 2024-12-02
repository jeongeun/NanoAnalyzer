import os
import sys
import argparse
import logging

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

# Golden JSON URLs for luminosity masks
GOLDEN_JSONS = {
#    "2016": "https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions16/13TeV/Legacy_2016/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt",
#    "2017": "https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions17/13TeV/Legacy_2017/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt",
    "2018": "https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions18/13TeV/Legacy_2018/Cert_314472-325175_13TeV_Legacy2018_JSON.txt"
}

# Function to parse input file
def parse_input_file(file_path):
    try:
        with open(file_path, 'r') as file:
            lines = [line.strip() for line in file if line.strip()]
    except FileNotFoundError:
        logging.error(f"File not found: {file_path}")
        sys.exit(1)
    datasets = []
    for idx, line in enumerate(lines):
        if ',' not in line:
            logging.warning(f"Invalid format in line {idx + 1}: '{line}'. Skipping.")
            continue
        script_name, script_path = line.split(',', 1)
        datasets.append((script_name.strip(), script_path.strip()))
    return datasets

# Function to create directory safely
def create_directory(path):
    try:
        os.makedirs(path, exist_ok=True)
        logging.info(f"Created directory: {path}")
    except OSError as e:
        logging.error(f"Failed to create directory {path}: {e}")
        sys.exit(1)

# Function to write a script file
def write_script_file(file_path, content):
    try:
        with open(file_path, 'w') as file:
            file.write(content)
        logging.info(f"Generated script: {file_path}")
    except OSError as e:
        logging.error(f"Failed to write file {file_path}: {e}")
        sys.exit(1)

def generate_crab_scripts(dataset_name, dataset_path, base_output_directory):
    # Prepare directories
    process_name = dataset_name
    process_base_name = os.path.splitext(dataset_name)[0]
    full_output_directory = os.path.join(base_output_directory, process_base_name)
    create_directory(full_output_directory)

    # File paths
    crab_cfg_path = os.path.join(full_output_directory, f"crab_cfg_{process_base_name}.py")
    crab_script_path = os.path.join(full_output_directory, "crab_script.py")
    crab_shell_path = os.path.join(full_output_directory, "crab_script.sh")
    fake_pset_path = os.path.join(full_output_directory, "PSet.py")

    # CRAB configuration content
    crab_cfg_content = f"""# Auto-generated CRAB configuration
from WMCore.Configuration import Configuration

config = Configuration()
config.section_("General")
config.General.requestName = '{process_name}'
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'PSet.py'
config.JobType.scriptExe = 'crab_script.sh'
config.JobType.inputFiles = ['crab_script.py', '../../ULskim_keep_and_drop.txt']

config.section_("Data")
config.Data.inputDataset = '{dataset_path}'
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.outLFNDirBase = '/store/user/[NAME]/[YOURDIRNAME]/{process_base_name}'
config.Data.publication = False
config.Data.outputDatasetTag = '{process_name}'

config.section_("Site")
config.Site.storageSite = "T3_KR_KNU"
"""

    # Add lumiMask if the dataset corresponds to a specific year
    for year, url in GOLDEN_JSONS.items():
        if year in process_name:
            crab_cfg_content = crab_cfg_content.replace(
                f"config.Data.inputDataset = '{dataset_path}'",
                f"config.Data.inputDataset = '{dataset_path}'\nconfig.Data.lumiMask = '{url}'"
            )

    # Crab script content
    crab_script_content = """#!/usr/bin/env python3
import os
from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import *

from PhysicsTools.NanoAODTools.postprocessing.utils.crabhelper import inputFiles, runsAndLumis

p = PostProcessor(".",
                  inputFiles(),
                  branchsel="ULskim_keep_and_drop.txt",
                  provenance=True,
                  fwkJobReport=True,
                  jsonInput=runsAndLumis())
p.run()

print("DONE")
"""

    # Shell script content
    crab_shell_content = """#!/bin/bash
echo "Check if TTY"
if [ "`tty`" != "not a tty" ]; then
  echo "DO NOT RUN THIS IN INTERACTIVE MODE, IT DELETES LOCAL FILES"
else
  echo "ENVIRONMENT"
  env
  echo "VOMS PROXY INFO"
  voms-proxy-info -all
  echo "STARTING CRAB SCRIPT"
  python3 crab_script.py $1
fi
"""

    # Fake PSet content
    fake_pset_content = """# Fake PSet needed for CRAB
import FWCore.ParameterSet.Config as cms

process = cms.Process('NANO')
process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring()
)
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(-1))
process.output = cms.OutputModule(
    "PoolOutputModule",
    fileName=cms.untracked.string('tree.root')
)
process.out = cms.EndPath(process.output)
"""

    # Write scripts
    write_script_file(crab_cfg_path, crab_cfg_content)
    write_script_file(crab_script_path, crab_script_content)
    write_script_file(crab_shell_path, crab_shell_content)
    write_script_file(fake_pset_path, fake_pset_content)

# Main function to process the input
def main():
    parser = argparse.ArgumentParser(description="Generate CRAB scripts for NanoAOD processing.")
    parser.add_argument("file_with_names_and_paths", help="Input file containing dataset names and paths.")
    parser.add_argument("base_output_directory", help="Base output directory for generated scripts.")
    args = parser.parse_args()

    datasets = parse_input_file(args.file_with_names_and_paths)
    for dataset_name, dataset_path in datasets:
        generate_crab_scripts(dataset_name, dataset_path, args.base_output_directory)

if __name__ == "__main__":
    main()
