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

from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection
from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module

class CutNCountProducer(Module):
    def __init__(self, isdata=False, era=2023, output_file="cutflow.txt", plot_file="cutflow.root"):
        self.isdata = isdata
        self.era = era
        self.output_file = output_file
        self.plot_file = plot_file
        self.total_events = 0
        self.cut1_noisefil_passed_events = 0
        self.cut2_trigger_passed_events = 0
        self.cut3_met120_passed_events = 0
        self.cut4_ele130_passed_events = 0
        self.cut5_heep_passed_events = 0
        self.cut6_2lveto_loose_passed_events = 0
        self.cut6_2lveto_loose_heep_passed_events = 0
        self.cut7_dphi_passed_events = 0
        self.cut8_energy_balance_passed_events = 0

        self.hlt_SingleEle = {
            2018: ["HLT_Photon200"],
            2022: ["HLT_Ele115_CaloIdVT_GsfTrkIdT", "HLT_Photon200"],
            2023: ["HLT_Ele115_CaloIdVT_GsfTrkIdT", "HLT_Photon200"]
        }

        # Initialize a histogram for cutflow
        self.cutflow_hist = ROOT.TH1F("cutflow", "Cutflow", 9, 0, 9)  # 9 bins for 9 cuts
        self.cutflow_labels = [
            "Total Events", "Noise Filters", "Trigger", "MET>120",
            "Ele_pT130", "HEEP", "2ndl_veto",
            "dPhi>2.5", "0.4<epT/MET<1.5"
        ]


    def beginJob(self):
        # Set bin labels for the cutflow histogram
        for i, label in enumerate(self.cutflow_labels):
            self.cutflow_hist.GetXaxis().SetBinLabel(i + 1, label)

    def endJob(self):
        with open(self.output_file, "w") as f:
            f.write(f"Total events before selection: {self.total_events}\n")
            f.write(f"Total events after cut1_noisefil selection: {self.cut1_noisefil_passed_events}, ({self.cut1_noisefil_passed_events/self.total_events})\n")
            f.write(f"Total events after cut2_trigger selection: {self.cut2_trigger_passed_events}, ({self.cut2_trigger_passed_events/self.total_events})\n")
            f.write(f"Total events after cut3_met120 selection: {self.cut3_met120_passed_events}, ({self.cut3_met120_passed_events/self.total_events})\n")
            f.write(f"Total events after cut4_ele130 selection: {self.cut4_ele130_passed_events}, ({self.cut4_ele130_passed_events/self.total_events})\n")
            f.write(f"Total events after cut5_heep selection: {self.cut5_heep_passed_events}, ({self.cut5_heep_passed_events/self.total_events})\n")
            f.write(f"Total events after cut6_2lveto_loose selection: {self.cut6_2lveto_loose_passed_events}, ({self.cut6_2lveto_loose_passed_events/self.total_events})\n")
            f.write(f"Total events after cut6_2lveto_loose_heep selection: {self.cut6_2lveto_loose_heep_passed_events}, ({self.cut6_2lveto_loose_heep_passed_events/self.total_events})\n")
            f.write(f"Total events after cut7 dphi cut: {self.cut7_dphi_passed_events}, ({self.cut7_dphi_passed_events/self.total_events})\n")
            f.write(f"Total events after cut8 energy balance cut: {self.cut8_energy_balance_passed_events}, ({self.cut8_energy_balance_passed_events/self.total_events})\n")


        # Fill the histogram with event counts
        self.cutflow_hist.SetBinContent(1, self.total_events)
        self.cutflow_hist.SetBinContent(2, self.cut1_noisefil_passed_events)
        self.cutflow_hist.SetBinContent(3, self.cut2_trigger_passed_events)
        self.cutflow_hist.SetBinContent(4, self.cut3_met120_passed_events)
        self.cutflow_hist.SetBinContent(5, self.cut4_ele130_passed_events)
        self.cutflow_hist.SetBinContent(6, self.cut5_heep_passed_events)
        self.cutflow_hist.SetBinContent(7, self.cut6_2lveto_loose_heep_passed_events)
        self.cutflow_hist.SetBinContent(8, self.cut7_dphi_passed_events)
        self.cutflow_hist.SetBinContent(9, self.cut8_energy_balance_passed_events)

        # Save the histogram to a ROOT file
        output_root = ROOT.TFile(self.plot_file, "RECREATE")
        self.cutflow_hist.Write()
        output_root.Close()

    def dPhi(self, phi1, phi2):
        """Calculate deltaPhi between two objects"""
        dphi = math.fabs(phi1 - phi2)
        if dphi > math.pi:
            dphi = 2 * math.pi - dphi
        return dphi

    def deltaR(self, obj1, obj2):
        """Calculate deltaR between two objects"""
        delta_phi = math.fabs(obj1.phi - obj2.phi)
        if delta_phi > math.pi:
            delta_phi = 2 * math.pi - delta_phi
        delta_eta = obj1.eta - obj2.eta
        return math.sqrt(delta_eta**2 + delta_phi**2)

    def check_noise_filters(self, event):
        try:
            return (
                event.Flag_goodVertices and
                event.Flag_globalSuperTightHalo2016Filter and
                event.Flag_BadPFMuonFilter and
                event.Flag_BadPFMuonDzFilter and
                event.Flag_EcalDeadCellTriggerPrimitiveFilter and
                event.Flag_eeBadScFilter and
                event.Flag_hfNoisyHitsFilter
            )
        except AttributeError:
            return False

    def analyze(self, event):
        """process event, return True (go to next module) or False (fail, go to next event)"""

        self.total_events += 1

        # Cut1: Noise filters
        if not self.check_noise_filters(event):
             return False
        self.cut1_noisefil_passed_events += 1

        # Cut2: HLT triggers
        passhltele = any(getattr(event, trig, False) for trig in self.hlt_SingleEle.get(self.era, []))
        if passhltele:
             self.cut2_trigger_passed_events += 1
        if not passhltele: return False

        # Cut3: offline MET cuts
        if not event.PuppiMET_pt > 120: return False
        self.cut3_met120_passed_events += 1

        # Cut4: offline electron pT 130 cuts
        isEle_pt130 = False
        isEle_pt130_heep = False

        electrons = Collection(event, "Electron")
        for iele in range(0, len(electrons)):
             if electrons[iele].pt >= 130 and abs(electrons[iele].eta + electrons[iele].deltaEtaSC) <= 2.5:
                   isEle_pt130 = True
                   break

        if isEle_pt130:
             self.cut4_ele130_passed_events += 1
        else:
             return False

        # Cut5: offline cuts+HEEP ID
        primary_ele =None

        for iele in range(0, len(electrons)):            
             if (electrons[iele].cutBased_HEEP):
                 isEle_pt130_heep = True
                 primary_ele = electrons[iele]
                 break

        if primary_ele is None: return False

        # Cut6: second lepton veto (2 case compare)
        if isEle_pt130_heep:
             self.cut5_heep_passed_events += 1
             is2ndEle_loose = False
             is2ndEle_loose_heep = False

             for ele in electrons:
                 if ele != primary_ele and ele.pt >= 25:
                     if ele.cutBased >= 2: #LooseID
                         is2ndEle_loose = True

                     if (ele.cutBased >= 2 or ele.cutBased_HEEP):
                         is2ndEle_loose_heep = True

                     break

             if not is2ndEle_loose:
                 self.cut6_2lveto_loose_passed_events += 1
             if not is2ndEle_loose_heep:
                 self.cut6_2lveto_loose_heep_passed_events += 1

        if is2ndEle_loose_heep: return False

        # Cut7,8: kinematic cuts dPhi, energy balanbce
        if self.dPhi(primary_ele.phi, event.PuppiMET_phi) > 2.5:
             self.cut7_dphi_passed_events += 1 
             if 0.4 < primary_ele.pt / event.PuppiMET_pt < 1.5:
                  self.cut8_energy_balance_passed_events += 1 

        return True

# Define module
CutNCountModuleConstr = lambda: CutNCountProducer(False, "2023", output_file="cutflow.txt", plot_file="cutflow.root")
