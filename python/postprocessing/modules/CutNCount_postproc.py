#!/usr/bin/env python3
#
# Example of running the postprocessor to skim events with a cut, and 
# adding a new variable using a Module.
#
from PhysicsTools.NanoAODTools.postprocessing.modules.CutNCountModule import *

from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor
from importlib import import_module
from CutNCountModule import *
import os
import sys
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

import argparse

parser = argparse.ArgumentParser()

parser.add_argument('-f',dest='infile',help="if an input file is not provide, assume this is a crab job")
parser.add_argument('-d',dest='isdata',action='store_true',default=False)
parser.add_argument('-y', dest='year', default='2023', help='year')
args = parser.parse_args()

# print arguments
print('Running with following configuration:')
for arg in vars(args):
    print('  - {}: {}'.format(arg,getattr(args,arg)))

if args.isdata:
    print ("Data, isdata = ", args.isdata)
else:
    print ("MC, isdata = ", args.isdata)

#cms-xrd-global.cern.ch
#cmsxrootd.fnal.gov
#xrootd-cms.infn.it

if args.infile:
    fnames = ["root://cms-xrd-global.cern.ch///store/mc/Run3Summer23NanoAODv12/WprimeToENu_M-1000_kR-1p0_LO_TuneCP5_13p6TeV_madgraph-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v15-v2/2520000/573b9781-fe39-4882-92bf-48ec8246c300.root",
"root://cms-xrd-global.cern.ch///store/mc/Run3Summer23NanoAODv12/WprimeToENu_M-1000_kR-1p0_LO_TuneCP5_13p6TeV_madgraph-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v15-v2/2520000/8b2abdb3-2dbd-4925-99f1-aaa33479d693.root"]

else:
    fnames = ["root://cms-xrd-global.cern.ch///store/mc/Run3Summer23NanoAODv12/WprimeToENu_M-1000_kR-1p0_LO_TuneCP5_13p6TeV_madgraph-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v15-v2/2520000/573b9781-fe39-4882-92bf-48ec8246c300.root",
"root://cms-xrd-global.cern.ch///store/mc/Run3Summer23NanoAODv12/WprimeToENu_M-1000_kR-1p0_LO_TuneCP5_13p6TeV_madgraph-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v15-v2/2520000/8b2abdb3-2dbd-4925-99f1-aaa33479d693.root"]

p = PostProcessor(outputDir=".",
                  inputFiles=fnames,
                  branchsel="Wp_branchsel.txt",
                  #cut="PuppiMET_pt >= 120",
                  modules=[CutNCountProducer()],
                  provenance=True,
                  maxEntries=20000, #just read the first maxEntries events
                  noOut=True,
                  #outputbranchsel = "Wp_outbranchsel.txt"
                  )
p.run()
