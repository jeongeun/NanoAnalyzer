## NanoAnalyzer

This repository allows collaborative development and analysis of NanoAOD datasets. Follow the steps below to clone, configure, and contribute to the repository.

---

### Introduction

This guide outlines the steps required to implement the analysis workflow specifically using skimmed nanoAOD datasets. 

It will be covers the process from running the tree analyzer to plotting distributions and generating templates for Combine.

This is no need CMSSW.

 `NanoDataLoader Class` provides the methods that are common to all analysis, such as the method to read a list of root files and form a chain. It also provides a method to read a list of selection cuts.

The class `NanoAnalyzer Class` (`include/NanoAnalyzer.h` and `src/NanoAnalyzer.cc`) inherits from `NanoDataLoader Class`.

The user's code should be placed in the method `Loop()` of `NanoAnalyzer Class`.

The main program (`src/Main.cc`) receives the configuration parameters (such as the input chain of root files and a file to provide a cut list) and executes the `NanoAnalyzer Class` code.

---

### Instructions

#### Environment Setting

Follow these steps to set up your environment and repositories: (worked in KNU_T3)

```bash
# Clone the necessary repositories
git clone https://github.com/jeongeun/NanoAnalyzer.git


# Change directory to NanoAnalyzer
cd NanoAnalyzer

# Run the environment setup (this sets up g++, ROOT, Python, and corrections)
./setup.sh
```
---

#### Execution Guide

To run the analysis, execute as following:

```bash
# Build and Install in CMake
./cmakeBuild.sh

# For the Locally test
cd test/

# Execute the analysis 
# It contains 10 arguments
#../install/bin/NanoAnalysis  {input.root or input.list}  {era}  {process name}  {isMC}  {DoPU}  {DoRocco}  {DoID}  {DoISO}  {DoHLT}  {output.root}
# Data example
../install/bin/NanoAnalysis /PATH/Muon_Run2022C/tree_0.root  2022preEE Muon_Run2022C 0 0 0 0 0 0 mu_data.root

# MC examples
../install/bin/NanoAnalysis /PATH/DYto2L-2Jets_MLL-50/tree_0.root  2022preEE DYto2Mu_MLL-120to200 1 0 0 0 0 0 dy.root

../install/bin/NanoAnalysis /PATH/DYto2L-2Jets_MLL-50/tree_0.root  2022preEE DYto2Mu_MLL-120to200 1 1 0 0 0 0 dy_pureweighting.root

../install/bin/NanoAnalysis /PATH/DYto2L-2Jets_MLL-50/tree_0.root  2022preEE DYto2Mu_MLL-120to200 1 1 1 0 0 0 dy_pu_rocco.root

../install/bin/NanoAnalysis /PATH/DYto2L-2Jets_MLL-50/tree_0.root  2022preEE DYto2Mu_MLL-120to200 1 1 1 1 1 1 dy_pu_rocco_id_iso_hlt.root

../install/bin/NanoAnalysis ../input/2022preEE/DYto2L-2Jets_MLL-50/DYto2L-2Jets_MLL-50_0.list  2022preEE DYto2Mu_MLL-120to200 1 1 1 1 1 1 dy_pu_rocco_id_iso_hlt.root

# check output histograms
root -l mu_data.root
```

---

#### Development Tips

---

#### Submitting batch Jobs on the Cluster

For condor job submission

```bash
# Move to the condorJob directory
cd condorJob/


# make working directories and condor scripts for each process/correction type
# usage: makeCondorScript.py [-h] -d outputDir [-e ERA] [-c {Base,PU,Rocco,ID,ISO,All}]
python3 makeCondorScript.py -d condorOut -e 2022preEE -c Base
python3 makeCondorScript.py -d condorOut -e 2022preEE -c PU
python3 makeCondorScript.py -d condorOut -e 2022preEE -c Rocco
python3 makeCondorScript.py -d condorOut -e 2022preEE -c ID
python3 makeCondorScript.py -d condorOut -e 2022preEE -c ISO
python3 makeCondorScript.py -d condorOut -e 2022preEE -c All

# do condor_submit at once
./submitall.sh condorOut/2022preEE

# check condor_status
condor_q 
condor_tail [id].[process]

# after finishing merge root files for every jobs
./hadd.sh  condorOut/2022preEE

```


More to be updated here.


---


### Analyzer related ref
- [PdmV Run-3](https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun3Analysis)
- [WorkBookMiniAOD](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD)
- [WorkBookNanoAOD](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookNanoAOD)
- [NanoAODTools (postproc)](https://github.com/cms-sw/cmssw/tree/master/PhysicsTools/NanoAODTools)
- [NanoAOD-wiki@gitlab](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/home)
- [NanoAOD Contents](https://cms-nanoaod-integration.web.cern.ch/autoDoc/)
- [CAT NANOTools](https://github.com/cms-cat/nanoAOD-tools-modules/tree/master)

### POG related refs
- [JsonPOG@gitlab](https://gitlab.cern.ch/cms-nanoAOD/jsonpog-integration)
- [Muon-wiki](https://muon-wiki.docs.cern.ch/)
- [MuonPOG](https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonPOG#References_for_advanced_users_an)
- [Muon-Rochcor](https://twiki.cern.ch/twiki/bin/view/CMS/RochcorMuon)
- [Muon-SF](https://twiki.cern.ch/twiki/bin/view/CMS/MuonRun32022)
- [EGMPOG](https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPOG)
- [EGM-SFnSS] (https://twiki.cern.ch/twiki/bin/view/CMS/EgammSFandSSRun3)
- [NoiseFilter](https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetMET#Run3_recommendations)
- [MET XY corr](https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETRun2Corrections#xy_Shift_Correction_MET_phi_modu)
- [JME POG](https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetMET#)
- [Jet ID](https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetID)
- [JES-JER-JetVetoMap](https://cms-jerc.web.cern.ch/Recommendations/)
- [Tau ID Run-3](https://twiki.cern.ch/twiki/bin/view/CMS/TauIDRecommendationForRun3)

### Useful refs
- [CMS Lumi Pub](https://twiki.cern.ch/twiki/bin/view/CMSPublic/LumiPublicResults)
- [Lumi Run-3](https://twiki.cern.ch/twiki/bin/view/CMS/LumiRecommendationsRun3)
- [JSON](https://cms-service-dqmdc.web.cern.ch/CAF/certification/)
- [PileupJSONforData](https://twiki.cern.ch/twiki/bin/view/CMS/PileupJSONFileforData)
- [SM Xsec at 13.6TeV](https://twiki.cern.ch/twiki/bin/viewauth/CMS/MATRIXCrossSectionsat13p6TeV)
- [LHC Higgs Xsec](https://twiki.cern.ch/twiki/bin/view/LHCPhysics/HiggsXSBR)
- [CMS CADI search](https://cms.cern.ch/iCMS/analysisadmin/cadilines)
- [iCMS-AN search](https://icms.cern.ch/tools/publications/notes/entries/AN/)
- [T2KNU-manual] (https://t2-cms.knu.ac.kr/wiki/index.php/HTCondor)


---

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




