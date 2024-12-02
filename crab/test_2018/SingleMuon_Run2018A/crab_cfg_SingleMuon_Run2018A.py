# Auto-generated CRAB configuration
from WMCore.Configuration import Configuration

config = Configuration()
config.section_("General")
config.General.requestName = 'SingleMuon_Run2018A'
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'PSet.py'
config.JobType.scriptExe = 'crab_script.sh'
config.JobType.inputFiles = ['crab_script.py', '../../ULskim_keep_and_drop.txt']

config.section_("Data")
config.Data.inputDataset = '/SingleMuon/Run2018A-UL2018_MiniAODv2_NanoAODv9_GT36-v1/NANOAOD'
config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions18/13TeV/Legacy_2018/Cert_314472-325175_13TeV_Legacy2018_JSON.txt'
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.outLFNDirBase = '/store/user/jelee/skimmingTest/SingleMuon_Run2018A'
config.Data.publication = False
config.Data.outputDatasetTag = 'SingleMuon_Run2018A'

config.section_("Site")
config.Site.storageSite = "T3_KR_KNU"
