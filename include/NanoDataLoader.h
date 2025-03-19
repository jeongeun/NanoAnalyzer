#ifndef NanoDataLoader_h
#define NanoDataLoader_h

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <memory> 
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TChain.h"
#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"

using namespace std;

class NanoDataLoader {
    private :
        string process;   //processName
        string DataEra;   //era
        string inputlist; //files and full paths
        Bool_t isMC;      //isMC

        TChain* fChain        = nullptr;
        TTreeReader* fReader  = nullptr;
        Long64_t nTotalEvents = 0;
        Bool_t isinit         = false;

    public :
        NanoDataLoader(const string& processName, const string& era, const string& inputFileList, Bool_t isMC)
        : process(processName), DataEra(era), inputlist(inputFileList), isMC(isMC)
        {};
        virtual ~NanoDataLoader();

        void Init();
        void Clear();
        void LoadBranches();
        void PrintInitInfo();

        Long64_t GetTotalEvents() { return nTotalEvents; }

        Bool_t ReadNextEntry(){
            if (!isinit) {
                cerr << "[ERROR] NanoDataLoader::ReadNextEntry() - Data is not initialized" << endl;
                return false;
            }
            return fReader->Next();
        }

        // Get Chain 
        TChain*      GetChain()  { return fChain; }
        TTreeReader* GetReader() { return fReader; }

        // Load NanoAOD Branches
        // Ref : https://cms-nanoaod-integration.web.cern.ch/autoDoc

        // Noise filter
        TTreeReaderValue<Bool_t>* Flag_goodVertices                       = nullptr;
        TTreeReaderValue<Bool_t>* Flag_globalSuperTightHalo2016Filter     = nullptr;
        TTreeReaderValue<Bool_t>* Flag_EcalDeadCellTriggerPrimitiveFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_BadPFMuonFilter                    = nullptr;
        TTreeReaderValue<Bool_t>* Flag_BadPFMuonDzFilter                  = nullptr;
        TTreeReaderValue<Bool_t>* Flag_hfNoisyHitsFilter                  = nullptr;
        TTreeReaderValue<Bool_t>* Flag_eeBadScFilter                      = nullptr;
        //TTreeReaderValue<Bool_t>* Flag_ecalBadCalibFilter                 = nullptr;
        //TTreeReaderValue<Bool_t>* Flag_HBHENoiseFilter                    = nullptr;
        //TTreeReaderValue<Bool_t>* Flag_HBHENoiseIsoFilter                 = nullptr;
        // TTreeReaderValue<Bool_t>* Flag_BadChargedCandidateFilter         = nullptr;
        // L1 pre-firing weight
        //TTreeReaderValue<Float_t>* L1PreFiringWeight_Nom = nullptr;

        // Primary Vertices
        TTreeReaderValue<UChar_t>* NPV          = nullptr;
        TTreeReaderValue<UChar_t>* NPV_npvsGood = nullptr;

        // HLT
        TTreeReaderValue<Bool_t>* HLT_Mu50          = nullptr;
        TTreeReaderValue<Bool_t>* HLT_CascadeMu100  = nullptr;
        TTreeReaderValue<Bool_t>* HLT_HighPtTkMu100 = nullptr;
        //TTreeReaderValue<Bool_t>* HLT_IsoMu24       = nullptr;
        //TTreeReaderValue<Bool_t>* HLT_IsoTkMu24     = nullptr;
        //TTreeReaderValue<Bool_t>* HLT_IsoMu27       = nullptr;

        // Muon info
        TTreeReaderValue<Int_t>*   nMuon               = nullptr;
        TTreeReaderArray<Float_t>* Muon_pt             = nullptr;
        TTreeReaderArray<Float_t>* Muon_tunepRelPt     = nullptr;
        TTreeReaderArray<Float_t>* Muon_eta            = nullptr;
        TTreeReaderArray<Float_t>* Muon_phi            = nullptr;
        TTreeReaderArray<Float_t>* Muon_mass           = nullptr;
        TTreeReaderArray<Int_t>*   Muon_charge         = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isGlobal       = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isStandalone   = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isTracker      = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isPFcand       = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_looseId        = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_mediumId       = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_tightId        = nullptr;
        TTreeReaderArray<UChar_t>* Muon_highPtId       = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_highPurity     = nullptr;
        TTreeReaderArray<UChar_t>* Muon_nTrackerLayers = nullptr;
        TTreeReaderArray<UChar_t>* Muon_nStations      = nullptr;
        TTreeReaderArray<Float_t>* Muon_tkRelIso       = nullptr;
        TTreeReaderArray<Float_t>* Muon_pfRelIso03_all = nullptr;
        TTreeReaderArray<Float_t>* Muon_pfRelIso03_chg = nullptr;
        TTreeReaderArray<Float_t>* Muon_pfRelIso04_all = nullptr;

        // MET and PuppiMET and DeepMET info
        TTreeReaderValue<Float_t>* PuppiMET_pt               = nullptr;
        TTreeReaderValue<Float_t>* PuppiMET_phi              = nullptr;
        TTreeReaderValue<Float_t>* PuppiMET_sumEt            = nullptr;
        TTreeReaderValue<Float_t>* MET_pt                    = nullptr;
        TTreeReaderValue<Float_t>* MET_phi                   = nullptr;
        TTreeReaderValue<Float_t>* MET_sumEt                 = nullptr;
        TTreeReaderValue<Float_t>* DeepMETResolutionTune_pt  = nullptr;
        TTreeReaderValue<Float_t>* DeepMETResolutionTune_phi = nullptr;
        TTreeReaderValue<Float_t>* DeepMETResponseTune_pt    = nullptr;
        TTreeReaderValue<Float_t>* DeepMETResponseTune_phi   = nullptr;

        // MC Branches

        // Generator-Level Weight
        TTreeReaderValue<Float_t>* GenWeight                = nullptr;
        TTreeReaderValue<Float_t>* LHEWeight_originalXWGTUP = nullptr;
        TTreeReaderArray<Float_t>* LHEPdfWeight             = nullptr;
        TTreeReaderArray<Float_t>* LHEScaleWeight           = nullptr;

        // Pileup info
        TTreeReaderValue<Int_t>*   Pileup_nPU      = nullptr;
        TTreeReaderValue<Float_t>* Pileup_nTrueInt = nullptr;

        // GenParticle info
        TTreeReaderValue<Int_t>*    nGenPart                 = nullptr; 
        TTreeReaderArray<Float_t>*  GenPart_eta              = nullptr;
        TTreeReaderArray<Float_t>*  GenPart_phi              = nullptr;
        TTreeReaderArray<Float_t>*  GenPart_pt               = nullptr;
        TTreeReaderArray<Float_t>*  GenPart_mass             = nullptr;
        TTreeReaderArray<Int_t>*    GenPart_pdgId            = nullptr;
        TTreeReaderArray<Int_t>*    GenPart_status           = nullptr;
        TTreeReaderArray<UShort_t>* GenPart_statusFlags      = nullptr;
        TTreeReaderArray<Short_t>*  GenPart_genPartIdxMother = nullptr;

        // LHEParticle info
        TTreeReaderValue<Int_t>*  nLHEPart        = nullptr;
        TTreeReaderArray<Float_t>* LHEPart_pt      = nullptr;
        TTreeReaderArray<Float_t>* LHEPart_eta     = nullptr;
        TTreeReaderArray<Float_t>* LHEPart_phi     = nullptr;
        TTreeReaderArray<Float_t>* LHEPart_mass    = nullptr;
        TTreeReaderArray<Int_t>*   LHEPart_pdgId   = nullptr;
        TTreeReaderArray<Int_t>*   LHEPart_status  = nullptr;
        TTreeReaderArray<Int_t>*   LHEPart_spin    = nullptr;
        TTreeReaderArray<Float_t>* LHE_Vpt         = nullptr;
        TTreeReaderValue<Float_t>* LHE_HT          = nullptr;
        TTreeReaderValue<Float_t>* LHE_HTIncoming  = nullptr;

        // GenMET info
        TTreeReaderArray<Float_t>* GenMET_pt  = nullptr;
        TTreeReaderArray<Float_t>* GenMET_phi = nullptr;
};

#endif
