#ifndef EfficiencySF_h
#define EfficiencySF_h

// C++ classes
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <map>

// ROOT classes
#include "TFile.h"
#include "TH2F.h"
#include "TAxis.h"
#include "TH1.h"
#include "correction.h"
#include "Muon.h"
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;
using correction::CorrectionSet;

class EfficiencySF {
    private :
        
        Bool_t isinit = false;
        string DataEra;

        string subDirName;
        string Pileup_key;
        string fileNameLUM;
        string MuonID_key;
        string MuonISO_key;
        string MuonHLT_key;
        string fileNameMUO;
        unique_ptr<correction::CorrectionSet> muonSF;
        //inline const map<TString, array<TString, 3>> json_muon2022 = {
        //    { "2022preEE" ,  {"MUO/2022_27Jun2023/muon_JPsi.json", "MUO/2022_27Jun2023/muon_Z.json", "MUO/2022_27Jun2023/muon_HighPt.json"}},
        //    { "2022postEE",  {"MUO/2022EE_27Jun2023/muon_JPsi.json", "MUO/2022EE_27Jun2023/muon_Z.json", "MUO/2022EE_27Jun2023/muon_HighPt.json"}},
        //};

        ///*--------
        TFile* fFile_ID = nullptr;
        TFile* fFile_Iso = nullptr;
        TFile* fFile_Trig = nullptr;

        TH2F* hHist_ID = nullptr;
        TH2F* hHist_Iso = nullptr;
        TH2F* hHist_Trig = nullptr;

        TAxis* tAxis_ID_pt    = nullptr;
        TAxis* tAxis_ID_eta   = nullptr;
        TAxis* tAxis_Iso_pt   = nullptr;
        TAxis* tAxis_Iso_eta  = nullptr;
        TAxis* tAxis_Trig_pt  = nullptr;
        TAxis* tAxis_Trig_eta = nullptr;

        Double_t dValidPtMin_ID;
        Double_t dValidPtMax_ID;
        Double_t dValidEtaMin_ID;
        Double_t dValidEtaMax_ID;

        Double_t dValidPtMin_Iso;
        Double_t dValidPtMax_Iso;
        Double_t dValidEtaMin_Iso;
        Double_t dValidEtaMax_Iso;
        
        Double_t dValidPtMin_Trig;
        Double_t dValidPtMax_Trig;
        Double_t dValidEtaMin_Trig;
        Double_t dValidEtaMax_Trig;
       //--------*/

        string hName_ID;
        string hName_Iso;
        string hName_Trig;

    public :                
        EfficiencySF(const string& era, const string& histName_ID, const string& histName_Iso, const string& histName_Trig) :
            DataEra(era), hName_ID(histName_ID), hName_Iso(histName_Iso), hName_Trig(histName_Trig) {};
        virtual ~EfficiencySF();

        void SetEra(const string& era) {DataEra = era;}
        void SetHistName(const string& histName_ID, const string& histName_Iso, const string& histName_Trig) {
            hName_ID = histName_ID;
            hName_Iso = histName_Iso;
            hName_Trig = histName_Trig;
        }

        void Init();
        void Clear();
        void PrintInitInfo();


        vector<Double_t> GetMuonSF(Double_t minpt, Double_t pt, Double_t eta, string sys_weight);
        vector<Double_t> GetMuonIDSF(vector<MuonHolder>& muonCollection, Double_t minpt, string sys_weight);
        //vector<Double_t> GetEfficiency(Double_t pt, Double_t eta);

};

#endif
