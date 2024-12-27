# This is an example of a NanoAODTools Module to add one variable to nanoAODs.
# Note that:
# -the new variable will be available for use in the subsequent modules
# -it is possible to update the value for existing variables
#
# Example of using from command line:
# nano_postproc.py outDir /eos/cms/store/user/andrey/f.root -I PhysicsTools.NanoAODTools.postprocessing.examples.exampleModule exampleModuleConstr
#
# Example of running in a python script: see test/example_postproc.py
#
#!/usr/bin/env python
# Analyzer for WZG Analysis based on nanoAOD tools

import os, sys
import math
import ROOT

from importlib import import_module
from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor

#from PhysicsTools.NanoAODTools.postprocessing.tools import deltaR
from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection
from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module

class TnPProducer(Module):
    def __init__(self, isdata=False, era=2018):
        self.isdata = isdata
        self.era = era
        self.hlt_SingleEle = {
            2017: ["HLT_Ele27_WPTight_Gsf", "HLT_Ele35_WPTight_Gsf", "HLT_Ele38_WPTight_Gsf"],
            2018: ["HLT_Ele32_WPTight_Gsf"],
            2022: ["HLT_Ele35_WPTight_Gsf"],
            2023: ["HLT_Ele35_WPTight_Gsf"]
        }

    def beginFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        self.out = wrappedOutputTree
        self.out.branch("tag_charge", "F")
        self.out.branch("tag_pt", "F")
        self.out.branch("tag_eta", "F")
        self.out.branch("tag_phi", "F")
        self.out.branch("ntags", "i")
        self.out.branch("probe_charge", "F")
        self.out.branch("probe_pt", "F")
        self.out.branch("probe_eta", "F")
        self.out.branch("probe_phi", "F")
        self.out.branch("nprobes", "i")
        self.out.branch("tnp_mass", "F")
        self.out.branch("ntnp_pairs", "i")
        self.out.branch("probe_passing_q", "F")
        self.out.branch("probe_passing_pt", "F")
        self.out.branch("probe_passing_eta", "F")
        self.out.branch("probe_passing_phi", "F")
        self.out.branch("nprobes_passing", "i")
        self.out.branch("hlt_SingleEle_fired", "O")


    def endFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        pass

    def analyze(self, event):
        """process event, return True (go to next module) or False (fail, go to next event)"""
#        if not event.Flag_goodVertices : return False # Good PV filter
        def deltaR(self, obj1, obj2):
            """Calculate deltaR between two objects"""
            delta_phi = math.fabs(obj1.phi - obj2.phi)
            if delta_phi > math.pi:
                delta_phi = 2 * math.pi - delta_phi
            delta_eta = obj1.eta - obj2.eta
            return math.sqrt(delta_eta**2 + delta_phi**2)

        #### set variables ###
        tags_list, probes_list = [], []
        tags, probes, tnp_pairs = [], [], []
        probes_passing = []
        #trig_objs = Collection(event, "TrigObj")
        # Initialize all output branches to default values
        self.out.fillBranch("ntags", 0)
        self.out.fillBranch("nprobes", 0)
        self.out.fillBranch("ntnp_pairs", 0)
        self.out.fillBranch("nprobes_passing", 0)
        self.out.fillBranch("tag_charge", 0.0)
        self.out.fillBranch("tag_pt", 0.0)
        self.out.fillBranch("tag_eta", 0.0)
        self.out.fillBranch("tag_phi", 0.0)
        self.out.fillBranch("probe_charge", 0.0)
        self.out.fillBranch("probe_pt", 0.0)
        self.out.fillBranch("probe_eta", 0.0)
        self.out.fillBranch("probe_phi", 0.0)
        self.out.fillBranch("tnp_mass", 0.0)
        self.out.fillBranch("probe_passing_q", 0.0)
        self.out.fillBranch("probe_passing_pt", 0.0)
        self.out.fillBranch("probe_passing_eta", 0.0)
        self.out.fillBranch("probe_passing_phi", 0.0)
        self.out.fillBranch("hlt_SingleEle_fired", 0)

        # Check HLT triggers
        passhltele = any(getattr(event, trig, False) for trig in self.hlt_SingleEle.get(self.era, []))
        if passhltele is None:
            sys.exit(f"ERROR: era not supported: {self.era}")
        self.out.fillBranch("hlt_SingleEle_fired", passhltele)


        # tag selection criteria : must be in barrel and pass tight id
        electrons = Collection(event, "Electron")
        for ele in electrons:
            if ele.pt < 15 or abs(ele.eta + ele.deltaEtaSC) > 2.5:
                continue
            probes_list.append(ele)
            if abs(ele.eta + ele.deltaEtaSC) < 1.479 and ele.cutBased >= 4:  # Tight ID
                tags_list.append(ele) #save tag candidate into tags
            else: 
                continue 

        if len(tags_list) < 1: 
            return False
        self.out.fillBranch("ntags", len(tags_list))

        # Probe selection : 50 < tnp Z mass < 130 GeV
        for tag in tags_list:
            for probe in probes_list:
                if probe is tag:  # Avoid using tag itself as a probe
                    continue
                if tag.charge * probe.charge > 0:  # Opposite charge requirement
                    continue
                tnp_pair_mass = (tag.p4() + probe.p4()).M()
                if (50 <= tnp_pair_mass <= 130):
                    if tag not in tags:
                        tags.append(tag)
                    if probe not in probes:
                        probes.append(probe)
                    tnp_pairs.append((tag, probe))

                    # Fill branches
                    self.out.fillBranch("tag_charge"  , tag.charge  )
                    self.out.fillBranch("tag_pt"      , tag.pt      )
                    self.out.fillBranch("tag_eta"     , tag.eta     )
                    self.out.fillBranch("tag_phi"     , tag.phi     )
                    self.out.fillBranch("probe_charge", probe.charge)
                    self.out.fillBranch("probe_pt"    , probe.pt    )
                    self.out.fillBranch("probe_eta"   , probe.eta   )
                    self.out.fillBranch("probe_phi"   , probe.phi   )
                    self.out.fillBranch("tnp_mass"    , tnp_pair_mass)

                    if passhltele:
                        probes_passing.append(probe)
                        self.out.fillBranch("probe_passing_q"   , probe.charge)
                        self.out.fillBranch("probe_passing_pt"  , probe.pt    )
                        self.out.fillBranch("probe_passing_eta" , probe.eta   )
                        self.out.fillBranch("probe_passing_phi" , probe.phi   )
                    # trigger matching
                    #for trig_obj in trig_objs:
                    #    if trig_obj.id == 11 and trig_obj.filterBits & 2:  # Trigger condition´
                    #        if deltaR(trig_obj, probe) < 0.3:  # DeltaR matching
                    #        probes_passing.add(probe)
                    #        break
        self.out.fillBranch("nprobes", len(probes))
        self.out.fillBranch("ntnp_pairs", len(tnp_pairs))
        self.out.fillBranch("nprobes_passing", len(probes_passing))
        return True

# Define module
TnPPModuleConstr = lambda: TnPProducer(False, "2018")

