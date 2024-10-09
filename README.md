# NanoAnalyzer Framework Guide

This repository allows collaborative development and analysis of NanoAOD datasets. Follow the steps below to clone, configure, and contribute to the repository.

## How to Collaborate in the NanoAnalyzer Repository

1. **Fork the NanoAnalyzer repository**: 
   In your first time, fork this NanoAnalyzer repository by clicking the "Fork" button on the top right corner of the GitHub page (https://github.com/jeongeun/NanoAnalyzer/).

2. **Clone your forked NanoAnalyzer repository**:
   Clone your forked repository to your local machine. For example:
   
   ```bash
   # Clone the repository
   git clone git@github.com:<YourGitAccount>/NanoAnalyzer.git
   # If you need submodules, use:
   git clone --recurse-submodules git@github.com:<YourGitAccount>/NanoAnalyzer.git
   ```
3. **Add upstream to stay updated with the central repository**:
   Regularly pull changes from the upstream repository to stay updated with the latest code and avoid conflicts.
   ```bash
   # Add the upstream remote (central repository)
   git remote add upstream git@github.com:jeongeun/NanoAnalyzer.git
   ```
4. **Make your changes on the development branch**:
Checkout your development branch and modify or update your code
   ```bash
   # Switch to your development branch (create one if necessary)
   git checkout -b <YourBranchName>
   # Edit and test your code
   vi test.py
   ```
5. **Make a pull request (PR) to the central NanoAnalyzer repository with your committed changes**:
   ```bash
   git add test.py
   git commit -m "Create test.py for making skimmed (small-size) NanoAOD MC"
   git push origin <YourBranchName>
   # Now, go to GitHub and make a PR from your branch
   ```

------
# How to use NanoAOD Tools (in python3)
Based on https://github.com/cms-sw/cmssw/tree/master/PhysicsTools/NanoAODTools

A simple set of python tools to post-process NanoAODs to:
- skim events
- add variables
- produce plots
- perform simple analyses (but beware that performance may be unsatisfactory beacuse of the inherently sequential design model).
It can be used directly from a CMSSW environment, or checked out as a standalone package.

## CMSSW release installation
First, install your favorite CMSSW release. If you rely on correctionlib and python3, it is strongly recommend to use CMSSW 11.3 or newer (https://github.com/cms-cat/nanoAOD-tools-modules). For example 
```bash
export SCRAM_ARCH=slc7_amd64_gcc900
cmsrel CMSSW_11_3_4
cd CMSSW_11_3_4/src
cmsenv
```
## Install nanoAOD-tools and NATModules
Install nanoAOD-tools and NATModules to process nanoAOD files and compile (build) everything.
Note that starting from CMSSW 13_0_16, 13_1_5 and 13_3_0, a basic version of nanoAOD-tools is included. To install the standalone version, please do
```bash
cd $CMSSW_BASE/src/
git clone https://github.com/cms-nanoAOD/nanoAOD-tools.git PhysicsTools/NanoAODTools
git clone git@github.com:cms-cat/nanoAOD-tools-modules.git PhysicsTools/NATModules
scram b
````
## install the correctionlib 
correction (SF) JSON files (https://github.com/cms-nanoAOD/correctionlib) provided by CMS into PhysicsTools/NATModules/data from the [cms-nanoAOD/jsonpog-integration](https://gitlab.cern.ch/cms-nanoAOD/jsonpog-integration) repository on GitLab.
Alternatively, this repository is regularly synchronized to /cvmfs/, so if your system has access, you can copy the latest version
```bash
cd $CMSSW_BASE/src/PhysicsTools/NATModules
git clone ssh://git@gitlab.cern.ch:7999/cms-nanoAOD/jsonpog-integration.git data
or
cd $CMSSW_BASE/src/PhysicsTools/NATModules
cp -r /cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration data
```
## Testing Run a module
To use in your own analysis, you can use the standalone scripts in [test](https://github.com/cms-cat/nanoAOD-tools-modules/tree/master/test) as an example. If you compiled this package correctly, you can import the modules in PhysicsTools/NATModules/python/modules as ```from PhysicsTools.NATModules.modules.muonSF import *```
```bash
cd $CMSSW_BASE/src/PhysicsTools/NATModules
python3 ./test/example_muonSF.py -i root://cms-xrd-global.cern.ch//store/mc/RunIISummer20UL16NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/20UL16JMENano_106X_mcRun2_asymptotic_v17-v1/2820000/11061525-9BB6-F441-9C12-4489135219B7.root
```
