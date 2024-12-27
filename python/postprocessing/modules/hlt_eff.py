#!/usr/bin/env python
# coding: utf-8
import sys
import os

import uproot as up
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep

plt.style.use(hep.style.CMS)

file = up.open("7B930101-EB91-4F4E-9B90-0861460DBD94_Skim.root")
infile = file["Events"]

branches = ["probe_pt", "probe_passing_pt"]
my_branch = infile.arrays(branches)

probe_passing = my_branch["probe_passing_pt"] # trigger pass
probe_total   = my_branch["probe_pt"] # total

bins = np.linspace(0,300,61) # 60 bins
bin_centers = (bins[:-1] + bins[1:]) / 2

h_total, _ = np.histogram(probe_total, bins=bins)
h_passing, _ = np.histogram(probe_passing, bins=bins)

h_total_np = ak.to_numpy(h_total)
h_passing_np = ak.to_numpy(h_passing)

plt.figure(figsize=(10, 8))
# CMS style labels
hep.cms.label("Preliminary", data=False, loc=0)

plt.hist(bin_centers, bins=bins, weights=h_total_np, histtype="step", label="Probe Total", color="blue", linestyle="-")
plt.hist(bin_centers, bins=bins, weights=h_passing_np, histtype="step", label="Probe Passing HLT_Ele35_WPTight_Gsf", color="red", linestyle="-")

# Labels and legend
plt.xlabel("Probe $p_T$ (GeV)")
plt.ylabel("Events")
plt.ylim(0, 1e+5)
plt.legend(loc="upper right")
plt.grid(visible=True)

# Save plot
plt.savefig("hist_probe.png")
plt.close()

efficiency = np.divide(
       h_passing_np, 
       h_total_np, 
       out=np.zeros_like(h_passing_np, dtype=float), 
       where=(h_total_np != 0)
)
#efficiency = np.nan_to_num(efficiency, nan=0.0, posinf=0.0, neginf=0.0)
efficiency_error = np.zeros_like(efficiency)
valid = h_total_np > 0
efficiency_error[valid] = np.sqrt(efficiency[valid] * (1 - efficiency[valid]) / h_total_np[valid])

plt.figure(figsize=(10, 8))
plt.errorbar(
    bin_centers,
    efficiency,
    #yerr=np.sqrt(efficiency * (1 - efficiency) / np.maximum(h_total_np, 1)),
    yerr=efficiency_error,
    fmt="o",
    color="black",
    label="HLT_Ele32_WPTight_Gsf",
)

# CMS style labels
hep.cms.label("Preliminary", data=False, loc=0)

# Labels and legend
plt.xlabel("Probe $p_T$ (GeV)")
plt.ylabel("HLT Efficiency")
plt.ylim(0, 1.2)
plt.legend(loc="upper right")
plt.grid(visible=True)

# Save plot
plt.savefig("hlt_ele32_eff.png")
plt.close()

#plt.show()
