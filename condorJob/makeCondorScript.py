import os
import glob
import argparse
from datetime import datetime  # Timestamp for log files

def makeScript(process, base_dir, corr, isMC, era, pu, rocco, idsf, iso, trig):

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_dir = os.path.join(base_dir, era, process, corr)
    os.makedirs(output_dir, exist_ok=True)

    log_dir = os.path.join(output_dir, "condorLog")
    os.makedirs(log_dir, exist_ok=True)
 
# It creates base output directory and sub-directories as follows:
# base_output_directory/
# -- era/         
# ---- process/   
# ------ correction config/    
# -------- run_process.sh
# -------- condor_process.sub
# -------- condorLog/

    list_dir     = f"/d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/input/{era}/{process}/"
    list_files   = sorted(glob.glob(os.path.join(list_dir, "*.list")))
    
    queue_ent = "queue arguments from (\n"
    queue_ent += "\n".join([f'    "{list} {i}"' for i, list in enumerate(list_files)])
    queue_ent += "\n)"


    run_script = f"""#!/bin/bash
source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-centos7-gcc12-opt/setup.sh 

cd /d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/install
export INSTALL_DIR_PATH=$PWD
export PATH="$PATH:$INSTALL_DIR_PATH/lib"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib"
#export CORRECTION_PATH="/cvmfs/sft.cern.ch/lcg/releases/correctionlib/2.2.2-18bbf/x86_64-centos7-gcc12-opt/include"
#export CORRECTION_LIBPATH="/cvmfs/sft.cern.ch/lcg/releases/correctionlib/2.2.2-18bbf/x86_64-centos7-gcc12-opt/lib"
#export JSONPOG_INTEGRATION_PATH="/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration"

cd /d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/install/bin

./NanoAnalysis ${1} {era} {process} {isMC} {pu} {rocco} {idsf} {iso} {trig} /d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/condorJob/{output_dir}/{process}_${2}.root
"""


    jdl_script = f"""universe = vanilla
executable = $ENV(PWD)/run_{process}.sh

arguments = $(InputFileList) $(Process)

request_memory = 2048 MB
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
use_x509userproxy = True

initialdir = $ENV(PWD)

transfer_input_files = /d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/RoccoR/RoccoR2018UL.txt, \
                       /d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/input/{era}/{process}.list
run_as_owner = True
JobBatchName = {era}.{process}.{corr}

output = ./condorLog/{process}_$(Process).log
error  = ./condorLog/{process}_$(Process).err
log    = /dev/null

{queue_ent}
"""
## In case you need to define some blacklist cluster
# Requirements = (Machine =!= "cluster291.knu.ac.kr") && (TARGET.Arch == "X86_64") && (TARGET.OpSys == "LINUX") && (TARGET.HasFileTransfer)
# '''

# creating the shell script files
    with open(os.path.join(output_dir, f"run_{process}.sh"), 'w') as file:
        file.write(run_script)
    print(f"Generated script: run_{process}.sh")

    with open(os.path.join(output_dir, f"job_{process}.jdl"), 'w') as file:
        file.write(jdl_script)
    print(f"Generated script: job_{process}.jdl")

# executing part in Main
# python3 makeCondorScript.py (-d ./condorOut  --era 2022preEE  --corr All) default setting
# python3 makeCondorScript.py -d /path/to/output/dir --era 2022preEE --config Base

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate Condor Scripts for NanoAnalysis")
    parser.add_argument("-d", "--dir", default="./condorOut", required=True, help="Base output directory")
    parser.add_argument("-e", "--era", default="2022preEE", help="Specify the data era")
    parser.add_argument("-c", "--corr", choices=["Base", "PU", "Rocco", "ID", "ISO", "All"], default="All", help="Configuration type")
    args = parser.parse_args()


    print (f"Running .. python3 makeCondorScript.py -d {args.dir} --era {args.era} --corr {args.corr}")
    corrs = {
        "Base":  (0, 0, 0, 0, 0),
        "PU":    (1, 0, 0, 0, 0),
        "Rocco": (1, 1, 0, 0, 0),
        "ID":    (1, 1, 1, 0, 0),
        "ISO":   (1, 1, 1, 1, 0),
        "All":   (1, 1, 1, 1, 1)
    }

    # Search Process
    list_file = f"/d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/input/list_{args.era}.txt"
    if not os.path.exists(list_file):
        print(f"[ERROR] List file {list_file} not found. Exiting.")
        exit(1)

    with open(list_file, "r") as file:
        processes = [line.strip() for line in file if line.strip()]

    pu, rocco, idsf, iso, trig = corrs[args.corr]
    for process in processes:
        isMC = 0 if "SingleMuon" in process or "Muon" in process else 1
        makeScript(process, args.dir, args.corr, isMC, args.era, pu, rocco, idsf, iso, trig)

