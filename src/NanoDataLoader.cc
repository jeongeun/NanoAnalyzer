#include "NanoDataLoader.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TSystem.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <memory> 

using namespace std;

void NanoDataLoader::Init() {
    if (isinit) {
        cerr << "[Warning] NanoDataLoader::Init() - Data is already initialized" << endl;
        return;
    }
    fChain = new TChain("Events");
    if (!fChain) {
        cerr << "[ERROR] NanoDataLoader::Init() - Failed to allocate TChain!" << endl;
        return;
    }
    // Check if inputlist is a .list file or a .root file
    if (inputlist.find(".list") != string::npos) {
        ifstream infile(inputlist);
        if (!infile) {
            throw runtime_error("[Runtime Error] NanoDataLoader::Init() - Cannot open input file list: " + inputlist);
        }
        cout << "-----------------------------------------------------------" << endl;
        cout << "[Info] NanoDataLoader::Init() - Adding files from .list to TChain" << endl;

        int fileCount = 0;
        int validFileCount = 0;
        string fline;

        while (getline(infile, fline)) {
            if (fline.empty() || fline.find_first_not_of(" \t\n\v\f\r") == string::npos) {
                cerr << "[Warning] Skipping empty or whitespace-only line in input file list." << endl;
                continue;
            }
            TString expandedPath = fline.c_str();
            gSystem->ExpandPathName(expandedPath);

            unique_ptr<TFile> testFile(TFile::Open(expandedPath, "READ"));
            if (!testFile || testFile->IsZombie()) {
                cerr << "[ERROR] Skipping damaged or invalid ROOT file: " << expandedPath << endl;
                continue;
            }

            TTree* testTree = dynamic_cast<TTree*>(testFile->Get("Events"));
            if (!testTree) {
                cerr << "[ERROR] Skipping ROOT file without 'Events' tree: " << expandedPath << endl;
                continue;
            }

            fChain->Add(expandedPath);
            validFileCount++;
            cout << "[Info] Added file " << validFileCount << ": " << expandedPath
                << " (Entries: " << testTree->GetEntries() << ")" << endl;

            fileCount++;
        }

        infile.close();
        if (validFileCount == 0) {
            cerr << "[ERROR] NanoDataLoader::Init() - No valid ROOT files were added." << endl;
            throw runtime_error("[ERROR] NanoDataLoader::Init() - Terminating due to no valid files.");
        }

        cout << "[Info] NanoDataLoader::Init() - Successfully added " << validFileCount << " valid file(s) to TChain." << endl;
        cout << "-----------------------------------------------------------" << endl;

    } else if (inputlist.find(".root") != string::npos) {
        // Input is a .root file
        TString expandedPath = inputlist.c_str();
        gSystem->ExpandPathName(expandedPath);

        unique_ptr<TFile> testFile(TFile::Open(expandedPath, "READ"));
        if (!testFile || testFile->IsZombie()) {
            cerr << "[ERROR] NanoDataLoader::Init() - Damaged or invalid ROOT file: " << expandedPath << endl;
            throw runtime_error("[ERROR] NanoDataLoader::Init() - Terminating due to invalid ROOT file.");
        }

        TTree* testTree = dynamic_cast<TTree*>(testFile->Get("Events"));
        if (!testTree) {
            cerr << "[ERROR] NanoDataLoader::Init() - ROOT file does not contain 'Events' tree: " << expandedPath << endl;
            throw runtime_error("[ERROR] NanoDataLoader::Init() - Terminating due to missing 'Events' tree.");
        }

        fChain->Add(expandedPath);
        cout << "[Info] NanoDataLoader::Init() - Successfully added ROOT file: " << expandedPath
             << " (Entries: " << testTree->GetEntries() << ")" << endl;

    } else {
        cerr << "[ERROR] NanoDataLoader::Init() - Unknown file format. Expected .list or .root" << endl;
        throw runtime_error("[ERROR] NanoDataLoader::Init() - Unknown file format.");
    }
    // Initialize TTreeReader
    fReader = new TTreeReader(fChain);
    if (!fReader) {
        throw runtime_error("[ERROR] NanoDataLoader::Init() - Failed to create TTreeReader");
    }

    this->LoadBranches();
    this->PrintInitInfo();

    isinit = true;
}


void NanoDataLoader::PrintInitInfo() {
    cout << "--------------------------------------------------------------" << endl;
    cout << "[Info] NanoDataLoader::PrintInitInfo() - Initializing ON   "    << endl;
    cout << "[Info] NanoDataLoader::PrintInitInfo() - Input file list: "     << inputlist    << endl;
    cout << "[Info] NanoDataLoader::PrintInitInfo() - Total Num of Events: " << nTotalEvents << endl;
    cout << "--------------------------------------------------------------" << endl;    
}

void NanoDataLoader::LoadBranches() {
    cout << "------------------------------------------------------------" << endl;
    cout << "[Info] NanoDataLoader::LoadBranches() - Loading Branches ON"  << endl;
    cout << "------------------------------------------------------------" << endl;

    ///////////////////////////////////////////////
    ////////// Branches for Data and MC ///////////
    ///////////////////////////////////////////////

    // NanoAOD Contents (This code was based on NanoAODv12, will be updated to v15 in 2025)
    // Ref: https://cms-nanoaod-integration.web.cern.ch/autoDoc/

    // Noise filter (must be applied for every analysis)
    // Ref: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2
    // 2022,2023 : https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#Run_3_2022_and_2023_data_and_MC
    // In Run-3, ECAL bad calibration filter ("Flag_ecalBadCalibFilter") have to be added manually.
    Flag_goodVertices                       = new TTreeReaderValue<Bool_t>(*fReader, "Flag_goodVertices");
    Flag_globalSuperTightHalo2016Filter     = new TTreeReaderValue<Bool_t>(*fReader, "Flag_globalSuperTightHalo2016Filter");
    Flag_EcalDeadCellTriggerPrimitiveFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_EcalDeadCellTriggerPrimitiveFilter");
    Flag_BadPFMuonFilter                    = new TTreeReaderValue<Bool_t>(*fReader, "Flag_BadPFMuonFilter");
    Flag_BadPFMuonDzFilter                  = new TTreeReaderValue<Bool_t>(*fReader, "Flag_BadPFMuonDzFilter");
    Flag_hfNoisyHitsFilter                  = new TTreeReaderValue<Bool_t>(*fReader, "Flag_hfNoisyHitsFilter");
    Flag_eeBadScFilter                      = new TTreeReaderValue<Bool_t>(*fReader, "Flag_eeBadScFilter");
    if (DataEra.find("2016") != string::npos) {
        //Flag_ecalBadCalibFilter             = new TTreeReaderValue<Bool_t>(*fReader, "Flag_ecalBadCalibFilter");
        //Flag_HBHENoiseFilter                = new TTreeReaderValue<Bool_t>(*fReader, "Flag_HBHENoiseFilter"); //Run2
        //Flag_HBHENoiseIsoFilter             = new TTreeReaderValue<Bool_t>(*fReader, "Flag_HBHENoiseIsoFilter"); //Run2
        //Flag_BadChargedCandidateFilter      = new TTreeReaderValue<Bool_t>(*fReader, "Flag_BadChargedCandidateFilter"); //Run2
    }
    // L1 pre-firing weight (This is for Run-2)
    //L1PreFiringWeight_Nom = new TTreeReaderValue<Float_t>(*fReader, "L1PreFiringWeight_Nom");

    // Primary Vertices (PV)
    NPV          = new TTreeReaderValue<UChar_t>(*fReader, "PV_npvs");//total num. of reconstructed PVs
    NPV_npvsGood = new TTreeReaderValue<UChar_t>(*fReader, "PV_npvsGood");//NPV pass (!isFake && ndof> 4 && abs(z)<=24 && position.Rho<=2)


    // HLT for high-pT muon
    //HLT_IsoMu24 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_IsoMu24");//not stored in skimmed nanoaod
    //HLT_IsoMu27 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_IsoMu27");//you can add by editing branchsel.txt
    HLT_Mu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");

    if (DataEra.find("2022") != string::npos) {
        HLT_Mu50             = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");
        HLT_CascadeMu100     = new TTreeReaderValue<Bool_t>(*fReader, "HLT_CascadeMu100");
        HLT_HighPtTkMu100    = new TTreeReaderValue<Bool_t>(*fReader, "HLT_HighPtTkMu100");
    }

    // Muons
    nMuon               = new TTreeReaderValue<Int_t>(  *fReader, "nMuon"); //slimmedMuons after basic sel (pt>15 || (pt>3 && anyID))
    Muon_pt             = new TTreeReaderArray<Float_t>(*fReader, "Muon_pt"); 
    Muon_tunepRelPt     = new TTreeReaderArray<Float_t>(*fReader, "Muon_tunepRelPt");//TuneP relative pt, tunePpt/pt
    Muon_eta            = new TTreeReaderArray<Float_t>(*fReader, "Muon_eta");
    Muon_phi            = new TTreeReaderArray<Float_t>(*fReader, "Muon_phi");
    Muon_mass           = new TTreeReaderArray<Float_t>(*fReader, "Muon_mass");
    Muon_charge         = new TTreeReaderArray<Int_t>(  *fReader, "Muon_charge");
    Muon_isGlobal       = new TTreeReaderArray<Bool_t>( *fReader, "Muon_isGlobal");
    Muon_isStandalone   = new TTreeReaderArray<Bool_t>( *fReader, "Muon_isStandalone");
    Muon_isTracker      = new TTreeReaderArray<Bool_t>( *fReader, "Muon_isTracker");
    Muon_isPFcand       = new TTreeReaderArray<Bool_t>( *fReader, "Muon_isPFcand");
    Muon_looseId        = new TTreeReaderArray<Bool_t>( *fReader, "Muon_looseId");
    Muon_mediumId       = new TTreeReaderArray<Bool_t>( *fReader, "Muon_mediumId");
    Muon_tightId        = new TTreeReaderArray<Bool_t>( *fReader, "Muon_tightId");
    Muon_highPtId       = new TTreeReaderArray<UChar_t>(*fReader, "Muon_highPtId"); //1=tk highpT, 2=global highpT, which includes tk highpT)
    Muon_highPurity     = new TTreeReaderArray<Bool_t>( *fReader, "Muon_highPurity"); //inner tk is high purity (Run2,3)
    Muon_nTrackerLayers = new TTreeReaderArray<UChar_t>(*fReader, "Muon_nTrackerLayers");
    Muon_nStations      = new TTreeReaderArray<UChar_t>(*fReader, "Muon_nStations");
    Muon_tkRelIso       = new TTreeReaderArray<Float_t>(*fReader, "Muon_tkRelIso"); //Tk-based rel isol dR=0.3 for highPt, trkIso/tunePpt
    Muon_pfRelIso03_all = new TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso03_all");
    Muon_pfRelIso03_chg = new TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso03_chg");
    Muon_pfRelIso04_all = new TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso04_all");
    
    // Missing Transverse momentum (MET)
    PuppiMET_pt               = new TTreeReaderValue<Float_t>(*fReader, "PuppiMET_pt");//Puppi based MET (Recommended to use in Run-3)
    PuppiMET_phi              = new TTreeReaderValue<Float_t>(*fReader, "PuppiMET_phi");
    PuppiMET_sumEt            = new TTreeReaderValue<Float_t>(*fReader, "PuppiMET_sumEt");
    MET_pt                    = new TTreeReaderValue<Float_t>(*fReader, "MET_pt");//PF based MET (default in Run-2)
    MET_phi                   = new TTreeReaderValue<Float_t>(*fReader, "MET_phi");
    MET_sumEt                 = new TTreeReaderValue<Float_t>(*fReader, "MET_sumEt");//scalar sum of Et
    DeepMETResolutionTune_pt  = new TTreeReaderValue<Float_t>(*fReader, "DeepMETResolutionTune_pt");//Deep based MET (need to test)
    DeepMETResolutionTune_phi = new TTreeReaderValue<Float_t>(*fReader, "DeepMETResolutionTune_phi");
    DeepMETResponseTune_pt    = new TTreeReaderValue<Float_t>(*fReader, "DeepMETResponseTune_pt");
    DeepMETResponseTune_phi   = new TTreeReaderValue<Float_t>(*fReader, "DeepMETResponseTune_phi");

    ///////////////////////////////////////////
    ////////// Branches for only MC ///////////
    ///////////////////////////////////////////

    if (isMC) {
        GenWeight                = new TTreeReaderValue<Float_t>(*fReader, "genWeight");

        // Pileup
        Pileup_nPU               = new TTreeReaderValue<Int_t>(  *fReader, "Pileup_nPU");//the number of pileup interactions that have been added to the event in the current bunch crossing
        Pileup_nTrueInt          = new TTreeReaderValue<Float_t>(*fReader, "Pileup_nTrueInt");//the true mean number of the poisson distribution for this event from which the number of interactions each bunch crossing has been sampled

        // GenParticle
        nGenPart                 = new TTreeReaderValue<Int_t>(  *fReader, "nGenPart");;
        GenPart_pt               = new TTreeReaderArray<Float_t>(*fReader, "GenPart_pt");
        GenPart_eta              = new TTreeReaderArray<Float_t>(*fReader, "GenPart_eta");
        GenPart_phi              = new TTreeReaderArray<Float_t>(*fReader, "GenPart_phi");
        GenPart_mass             = new TTreeReaderArray<Float_t>(*fReader, "GenPart_mass");
        GenPart_pdgId            = new TTreeReaderArray<Int_t>(  *fReader, "GenPart_pdgId");
        GenPart_status           = new TTreeReaderArray<Int_t>(  *fReader, "GenPart_status");//Particle status. 1=stable
        GenPart_statusFlags      = new TTreeReaderArray<UShort_t>(*fReader, "GenPart_statusFlags");
        //gen status flags stored bitwise, bits are: 0 : isPrompt, 1 : isDecayedLeptonHadron, 2 : isTauDecayProduct, 3 : isPromptTauDecayProduct, 4 : isDirectTauDecayProduct, 5 : isDirectPromptTauDecayProduct, 6 : isDirectHadronDecayProduct, 7 : isHardProcess, 8 : fromHardProcess, 9 : isHardProcessTauDecayProduct, 10 : isDirectHardProcessTauDecayProduct, 11 : fromHardProcessBeforeFSR, 12 : isFirstCopy, 13 : isLastCopy, 14 : isLastCopyBeforeFSR,
        GenPart_genPartIdxMother = new TTreeReaderArray<Short_t>( *fReader, "GenPart_genPartIdxMother");//index of the mother particle

        // GenMET
        GenMET_pt                = new TTreeReaderArray<Float_t>( *fReader, "GenMET_pt");
        GenMET_phi               = new TTreeReaderArray<Float_t>( *fReader, "GenMET_phi");

        // LHEPart (in case MG5,aMCatNLO samples)
        if (isMC && (process.find("WtoLNu-") != string::npos) ) {
            LHEPdfWeight             = new TTreeReaderValue<Float_t>(*fReader, "LHEPdfWeight");//PDF variation weights (w_var/w_nominal) for LHAPDF IDs 325300-325402     
            LHEScaleWeight           = new TTreeReaderValue<Float_t>(*fReader, "LHEScaleWeight");//Scale variation weights (w_var / w_nominal); [0] is MUF="0.5" MUR="0.5"; [1] is MUF="1.0" MUR="0.5"; [2] is MUF="2.0" MUR="0.5"; [3] is MUF="0.5" MUR="1.0"; [4] is MUF="2.0" MUR="1.0"; [5] is MUF="0.5" MUR="2.0"; [6] is MUF="1.0" MUR="2.0"; [7] is MUF="2.0" MUR="2.0" 
            LHEWeight_originalXWGTUP = new TTreeReaderValue<Float_t>(*fReader, "LHEWeight_originalXWGTUP"); //Nominal event weight in the LHE file
            nLHEPart                 = new TTreeReaderValue<UInt_t>( *fReader, "nLHEPart");
            LHEPart_pt               = new TTreeReaderValue<Float_t>(*fReader, "LHEPart_pt");
            LHEPart_eta              = new TTreeReaderValue<Float_t>(*fReader, "LHEPart_eta");
            LHEPart_phi              = new TTreeReaderValue<Float_t>(*fReader, "LHEPart_phi");
            LHEPart_mass             = new TTreeReaderValue<Float_t>(*fReader, "LHEPart_mass");
            LHEPart_pdgId            = new TTreeReaderValue<Int_t>(  *fReader, "LHEPart_pdgId");
            LHEPart_status           = new TTreeReaderValue<Int_t>(  *fReader, "LHEPart_status");//LHE particle status; -1:incoming, 1:outgoing
            LHEPart_spin             = new TTreeReaderValue<Int_t>(  *fReader, "LHEPart_spin");

            LHE_Vpt                  = new TTreeReaderValue<Float_t>(*fReader, "LHE_Vpt");//pT of the W or Z boson at LHE step
            LHE_HT                   = new TTreeReaderValue<Float_t>(*fReader, "LHE_HT");//HT, scalar sum of parton pTs at LHE step
            LHE_HTincoming           = new TTreeReaderValue<Float_t>(*fReader, "LHE_HTincoming");//HT, scalar sum of parton pTs at LHE step, restricted to partons
        }
    }

}

void NanoDataLoader::Clear() {
    // Clean up dynamically allocated members
    delete fChain;
    delete fReader;
    
    // Clean up all TTreeReader components
    delete Flag_goodVertices;
    delete Flag_globalSuperTightHalo2016Filter;
    delete Flag_EcalDeadCellTriggerPrimitiveFilter;
    delete Flag_BadPFMuonFilter;
    delete Flag_BadPFMuonDzFilter;
    delete Flag_hfNoisyHitsFilter;
    delete Flag_eeBadScFilter;
    //delete Flag_HBHENoiseFilter;
    //delete Flag_HBHENoiseIsoFilter;
    //delete Flag_BadChargedCandidateFilter;
    //delete Flag_ecalBadCalibFilter;
    //delete L1PreFiringWeight_Nom;
    //delete HLT_IsoMu24;
    delete HLT_Mu50;
    delete HLT_CascadeMu100;
    delete HLT_HighPtTkMu100;
    delete NPV;
    delete NPV_npvsGood;
    delete nMuon;
    delete Muon_pt;
    delete Muon_eta;
    delete Muon_phi;
    delete Muon_mass;
    delete Muon_nTrackerLayers;
    delete Muon_nStations;
    delete Muon_charge;
    delete Muon_tightId;
    delete Muon_mediumId;
    delete Muon_looseId;
    delete Muon_highPtId;
    delete Muon_isGlobal;
    delete Muon_isTracker;
    delete Muon_isPFcand;
    delete Muon_pfRelIso04_all;
    delete PuppiMET_pt;
    delete PuppiMET_phi;
    delete PuppiMET_sumEt;
    delete MET_pt;
    delete MET_phi;
    delete MET_sumEt;
    delete DeepMETResponseTune_pt;
    delete DeepMETResponseTune_phi;
    delete DeepMETResolutionTune_pt;
    delete DeepMETResolutionTune_phi;

    delete GenWeight;
    delete Pileup_nPU;
    delete Pileup_nTrueInt;
    delete nGenPart;
    delete GenPart_eta;
    delete GenPart_phi;
    delete GenPart_pt;
    delete GenPart_mass;
    delete GenPart_pdgId;
    delete GenPart_status;
    delete GenPart_statusFlags;
    delete GenPart_genPartIdxMother;
    delete LHEPart_pt;        
    delete LHEPart_eta;       
    delete LHEPart_phi;       
    delete LHEPart_mass;      
    delete LHEPart_pdgId;     
    delete LHEPart_status;    
    delete LHEPart_spin;      
    delete LHE_Vpt;       
    delete LHE_HT;        
    delete LHE_HTincoming;
    delete GenMET_phi;
    delete GenMET_pt;
}

NanoDataLoader::~NanoDataLoader() {
    Clear();
}
