#!/usr/bin/env python3
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
