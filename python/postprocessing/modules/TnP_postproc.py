#!/usr/bin/env python3
#
# Example of running the postprocessor to skim events with a cut, and 
# adding a new variable using a Module.
#
from PhysicsTools.NanoAODTools.postprocessing.modules.TnPModule import *

from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor
from importlib import import_module
from TnPModule import *
import os
import sys
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

import argparse

parser = argparse.ArgumentParser()

parser.add_argument('-f',dest='infile',help="if an input file is not provide, assume this is a crab job")
parser.add_argument('-d',dest='isdata',action='store_true',default=False)
parser.add_argument('-y', dest='year', default='2018', help='year')
args = parser.parse_args()

# print arguments
print('Running with following configuration:')
for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

if args.isdata:
    print ("Data, isdata = ", args.isdata)
else:
    print ("MC, isdata = ", args.isdata)


if args.infile:
    fnames = ["root://eoscms.cern.ch//eos/cms/store/user/cmsbuild/store/group/cat/datasets/NANOAODSIM/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v2/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/7B930101-EB91-4F4E-9B90-0861460DBD94.root"]
else:
    fnames = ["root://eoscms.cern.ch//eos/cms/store/user/cmsbuild/store/group/cat/datasets/NANOAODSIM/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v2/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/7B930101-EB91-4F4E-9B90-0861460DBD94.root"]

p = PostProcessor(outputDir=".",
                  inputFiles=fnames,
                  branchsel="branchsel.txt",#"Run2_UL_SkimList_MC.txt",
                  cut="Electron_pt > 15",
                  modules=[TnPProducer()],
                  provenance=True,
                  maxEntries=9, #just read the first maxEntries events
                  outputbranchsel = "outbranchsel.txt"
                  )
p.run()
