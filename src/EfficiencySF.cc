#include "EfficiencySF.h"
#include "correction.h"
#include <cmath>
#include <iostream>
#include <memory>

using namespace std;

EfficiencySF::~EfficiencySF() {
    Clear();
}

void EfficiencySF::Init() {
    if (isinit) {
        cerr << "[Warning] EfficiencySF::Init() - EfficiencySF is already initialized" << endl;
        return;
    }
  
    // MuonPOG corrections (signal muon in High pT (above 200 GeV))
    // https://gitlab.cern.ch/cms-nanoAOD/jsonpog-integration/-/tree/master/POG/MUO?ref_type=heads
    // https://twiki.cern.ch/twiki/bin/view/CMS/MuonRun32022
    string subDirName  = "" ;
    string MuonID_key  = "" ;
    string MuonISO_key = "" ;
    string MuonHLT_key = "" ;

    if (DataEra == "2016preVFP"){
        subDirName = "2016preVFP_UL";
        MuonHLT_key = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
    } 
    else if (DataEra == "2016postVFP"){
        subDirName = "2016postVFP_UL";
        MuonHLT_key = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
    } 
    else if (DataEra == "2017"){ 
        subDirName = "2017_UL";
        MuonHLT_key = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
    } 
    else if (DataEra == "2018"){
        subDirName = "2018_UL"; 
        MuonHLT_key = "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose";
    } else if (DataEra == "2022preEE"){ //this era is only confirmed now. please recheck the key for other eras.
        subDirName = "2022_Summer22";
        MuonID_key  = "NUM_HighPtID_DEN_TrackerMuons" ;
        MuonISO_key = "NUM_LooseRelTkIso_DEN_HighPtID" ;
        MuonHLT_key = "NUM_Mu50_or_CascadeMu100_or_HighPtTkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose";
        // root file MuonHLT_key = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
        // root file MuonHLT_key = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt";
    } 
    else if (DataEra == "2022postEE"){
        subDirName = "2022_Summer22EE";
        MuonHLT_key = "NUM_Mu50_or_CascadeMu100_or_HighPtTkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose";
    } 
    else if (DataEra == "2023preBPix"){
        subDirName = "2022_Summer23";
        MuonHLT_key = "NUM_Mu50_or_CascadeMu100_or_HighPtTkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose";
    } 
    else if (DataEra == "2023postBPix"){
        subDirName = "2022_Summer23BPix";
        MuonHLT_key = "NUM_Mu50_or_CascadeMu100_or_HighPtTkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose";
    } 
    else {
        cerr << "[ERROR] Unknown DataEra: " << DataEra << endl;
        return;
    }

    this->MuonID_key = MuonID_key;
    this->MuonISO_key = MuonISO_key;
    this->MuonHLT_key = MuonHLT_key;

    const string dirName = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/";

    this->fileNameMUO = dirName + "MUO/" + subDirName + "/muon_Z.json.gz";
    //this->fileNameMUO = dirName + "MUO/" + subDirName + "/muon_JPsi.json.gz";
    //this->fileNameMUO = dirName + "MUO/" + subDirName + "/muon_HighPt.json.gz";
    //this->fileNameMUO = dirName + "EGM/" + subDirName + "/electron.json.gz";

    // Load the correction file
    try {
        muonSF = correction::CorrectionSet::from_file(fileNameMUO);
        if (!muonSF) {
            throw runtime_error("Failed to load MUO correction file: " + fileNameMUO);
        }
    } catch (const exception& e) {
        cerr << "[ERROR] Failed to load Muon JSON file: " << fileNameMUO << endl;
        cerr << e.what() << endl;
        return;
    }

    PrintInitInfo();
    isinit = true;
}

void EfficiencySF::Clear() {
}

void EfficiencySF::PrintInitInfo() {
    cout << "----------------------------------------------------------" << endl;
    cout << "[Info] EfficiencySF::PrintInitInfo() - EfficiencySF is initialized" << endl;
    cout << "[Info] EfficiencySF::PrintInitInfo() - Era: "            << DataEra << endl;
    cout << "[Info] EfficiencySF::PrintInitInfo() - JSON File: "  << fileNameMUO << endl;
    cout << "[Info] EfficiencySF::PrintInitInfo() - MUON ID key: "  << MuonID_key << endl;
    cout << "[Info] EfficiencySF::PrintInitInfo() - MUON ISO key: "  << MuonISO_key << endl;
    cout << "[Info] EfficiencySF::PrintInitInfo() - MUON HLT key: "  << MuonHLT_key << endl;
    cout << "----------------------------------------------------------"   << endl;
}   

vector<Double_t> EfficiencySF::GetMuonSF(Double_t minpt, Double_t pt, Double_t eta, string sys_weight){
    if (!isinit) {
        cerr << "[ERROR] EfficiencySF::GetMuonSF() - Not initialized" << endl;
        return vector<Double_t> {1., 1., 1.};
    }
    if (!muonSF) {
        cerr << "[ERROR] muonSF is not loaded properly. Check file: " << this->fileNameMUO << endl;
        return vector<Double_t> {1., 1., 1.};
    }
    // Get the correction map
    auto map_SF_ID  = muonSF->at(this->MuonID_key);
    auto map_SF_ISO = muonSF->at(this->MuonISO_key);
    auto map_SF_HLT = muonSF->at(this->MuonHLT_key);
    //cout << "Get the Muon ID  Correction with KEY :" << this->MuonID_key  << endl;
    //cout << "Get the Muon ISO Correction with KEY :" << this->MuonISO_key << endl;
    //cout << "Get the Muon HLT Correction with KEY :" << this->MuonHLT_key << endl;

    if (!map_SF_ID) {
        cerr << "[ERROR] Correction map not found in file: " << this->fileNameMUO << endl;
        return vector<Double_t> {1., 1., 1.};
    }
    Double_t abs_eta = fabs(eta);
    std::vector<correction::Variable::Type> values = {abs_eta, pt, sys_weight};

    if (pt < minpt) {
        cout << "[INFO] SKIPPING beacasue Muon pt (" << pt << ") is below correction range (" << minpt << "). Setting sf weight to 1.0" << endl;
        return vector<Double_t> {1., 1., 1.};
    }

   Double_t sf_ID, sf_ISO, sf_HLT;

    try {
        sf_ID = map_SF_ID->evaluate(values);
        sf_ISO = map_SF_ISO->evaluate(values);
        sf_HLT = map_SF_HLT->evaluate(values);

        cout << "[INFO] EfficiencySF::GetMuonSF => Muon SF: pt = " << pt << ", eta = " << abs_eta << ", sys = " << sys_weight 
             << ", SF_ID,ISO,HLT = " << sf_ID << ", " << sf_ISO << ", " << sf_HLT << endl;    
        return vector<Double_t> {sf_ID, sf_ISO, sf_HLT} ;

    } catch (const exception& e) {
        cerr << "[ERROR] Failed to evaluate Muon Scale Factor" << endl;
        cerr << e.what() << endl;
        return vector<Double_t> {1., 1., 1.};
    }
}


vector<Double_t> EfficiencySF::GetMuonIDSF(vector<MuonHolder>& muonCollection, Double_t minpt, string sys_weight){
    if (!isinit) {
        cerr << "[ERROR] EfficiencySF::GetMuonIDSF() - Not initialized" << endl;
        return vector<Double_t> {1., 1., 1.};
    }
    vector<Double_t> sfs  = {1.0, 1.0, 1.0} ;
    for (MuonHolder& singleMuon : muonCollection){
        vector<Double_t> sfs = GetMuonSF(minpt, singleMuon.Pt(), singleMuon.Eta(), sys_weight);
        cout << "[INFO] Muon SF: pt = " << singleMuon.Pt() << ", eta = " << singleMuon.Eta() 
             << ", sys = " << sys_weight << ", SF = " << sfs[0] << ", " << sfs[1] << ", " << sfs[2] << endl;
    }
    return sfs;
}

