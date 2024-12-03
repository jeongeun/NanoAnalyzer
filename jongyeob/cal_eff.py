#!/usr/bin/env python
# coding: utf-8

# In[1]:


import sys
import os

import uproot4 as up
import numpy as np
import awkward as ak
import matplotlib.pyplot as plt
import mplhep as hep


# In[2]:


file = up.open("WZAToLNuLLA_4f_NanoAOD2022.root")
infile = file["Events"]

#tau_branches = [branch for branch in infile.keys() if branch.startswith("Tau")]
branches = ["Electron_pt","MET_phi","HLT_Ele32_WPTight_Gsf","HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8"]


# In[3]:


my_branch = infile.arrays(branches)


# In[4]:


emm_trigger = my_branch["HLT_Ele32_WPTight_Gsf"] | my_branch["HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8"]

print(emm_trigger) # combined trigger
print(np.sum(emm_trigger)) # number of triggered event

Electron_triggered = my_branch["Electron_pt"][emm_trigger] # trigger pass
Electron_pt = my_branch["Electron_pt"] # total


# In[5]:


print(ak.flatten(Electron_pt))

h0 = plt.hist(ak.flatten(Electron_pt),range=(0,300),bins=60,histtype='step',color='blue',label="untriggered")
h1 = plt.hist(ak.flatten(Electron_triggered),range=(0,300),bins=60,histtype='step',color='blue',label="untriggered")

eff = h1[0]/h0[0]
print(eff)


# In[6]:


plt.plot(h1[1][:-1],eff,marker='o',color='b',label="Trigger efficiency")

