# Fake PSet needed for CRAB
import FWCore.ParameterSet.Config as cms

process = cms.Process('NANO')
process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring()
)
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(-1))
process.output = cms.OutputModule(
    "PoolOutputModule",
    fileName=cms.untracked.string('tree.root')
)
process.out = cms.EndPath(process.output)
