#ifndef NanoAnalyzer_h
#define NanoAnalyzer_h

// NanoAnalyzer classes
#include "NanoDataLoader.h"
#include "PU.h"
#include "EfficiencySF.h"
#include "RoccoR.h"
#include "GenPart.h"
#include "Muon.h"
#include "MET.h"
// ROOT classes
#include "TRandom.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
// C++ classes
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <iomanip>
using namespace std;

class NanoAnalyzer {
    private :
        // Classes that will be created only once, at the beginning of the analysis
        NanoDataLoader* cData; // Class for loading Ntuple
        PU* cPU; // Class for loading PU files and calculating PU weights
        EfficiencySF* cEfficiencySF; // Class for loading efficiency SF files and calculating SFs
        RoccoR* cRochesterCorrection; // Class for loading Rochester correction file and applying correction
        
        // Flags for the class
        string inputlist;
        string DataEra;
        string process;
        string RoccorName;
        string hName_ID;
        string hName_Iso;
        string hName_Trig;

        string Muon_key;

        Bool_t isinit = false;
        Bool_t isMC = false;
        Bool_t bDoGenPatching = false;
        Bool_t bDoPUCorrection = false;
        //Bool_t bDoL1PreFiringCorrection = false;
        Bool_t bDoIDSF = false;
        Bool_t bDoIsoSF = false;
        Bool_t bDoTrigSF = false;
        Bool_t bDoRocco = false;

        // Check process name and determine whether to perform Gen-lv patching
        Bool_t bIsInclusiveW = false;
        Bool_t bIsBoostedW = false;
        Bool_t bIsOffshellW = false;
        Bool_t bIsOffshellWToTauNu = false;

        // Default HT cut and W mass cut
        Double_t dHT_cut_high = 1e9; //Default to infinity
        Double_t dW_mass_cut_low = 0.;
        Double_t dW_mass_cut_high = 1e9; //Default to infinity

        Long64_t nTotalEvents = 0;

        Double_t dSumOfGenEvtWeight = 0;

    public :
        NanoAnalyzer( const string& inputFileList, const string& processName, const string& era,
                    const string& HistName_ID, const string& HistName_Iso, const string& HistName_Trig,
                    const string& RoccoFileName,
                    Bool_t isMC, Bool_t doPUCorrection, //Bool_t doL1PreFiringCorrection, 
                    Bool_t bDoRocco, Bool_t bDoIDSF, Bool_t bDoIsoSF, Bool_t bDoTrigSF)
            :   inputlist(inputFileList), process(processName), DataEra(era), 
                hName_ID(HistName_ID), hName_Iso(HistName_Iso), hName_Trig(HistName_Trig),
                RoccorName(RoccoFileName),
                isMC(isMC), bDoPUCorrection(doPUCorrection), //bDoL1PreFiringCorrection(doL1PreFiringCorrection), 
                bDoRocco(bDoRocco), bDoIDSF(bDoIDSF), bDoIsoSF(bDoIsoSF), bDoTrigSF(bDoTrigSF)
        {};
        virtual ~NanoAnalyzer();

        // Initialize classes
        void Init();
        // Clear all pointers
        void Clear();
        // Declare histograms
        void DeclareHistograms();
        // Run event loop
        void Analyze();
        void Analyze_Z() {}; // TODO: For Z peak mass study
        // Print initialization information
        void PrintInitInfo();
        // Check process name and determine whether to perform Gen-lv patching
        void CheckGenPatching();
        // Simple utility to print progress
        void PrintProgress(const int currentStep);
        // Write histograms to file
        void WriteHistograms(TFile* f_output);

        // Getters
        Bool_t IsMC() {return isMC;}
        Bool_t DoPUCorrection() {return bDoPUCorrection;}
        //Bool_t DoL1PreFiringCorrection() {return bDoL1PreFiringCorrection;}
        Bool_t DoIDSF() {return bDoIDSF;}
        Bool_t DoIsoSF() {return bDoIsoSF;}
        Bool_t DoTrigSF() {return bDoTrigSF;}
        Bool_t DoRocco() {return bDoRocco;}

        Long64_t GetTotalEvents() {return nTotalEvents;}
        Double_t GetSumOfGenEvtWeight() {return dSumOfGenEvtWeight;}

        // Getters for classes
        NanoDataLoader& GetData() {return *cData;}
        PU& GetPU() {return *cPU;}
        EfficiencySF& GetEffSF() {return *cEfficiencySF;}
        RoccoR& GetRocco() {return *cRochesterCorrection;}

        // Setters
        void SetDoPUCorrection(Bool_t doPUCorrection) {bDoPUCorrection = doPUCorrection;}
        //void SetDoL1PreFiringCorrection(Bool_t doL1PreFiringCorrection) {bDoL1PreFiringCorrection = doL1PreFiringCorrection;}
        void SetDoIDSF(Bool_t doIDSF) {bDoIDSF = doIDSF;}
        void SetDoIsoSF(Bool_t doIsoSF) {bDoIsoSF = doIsoSF;}
        void SetDoTrigSF(Bool_t doTrigSF) {bDoTrigSF = doTrigSF;}
        void SetDoRocco(Bool_t bDoRocco) {bDoRocco = bDoRocco;}

        ////////////////////////////////////////////////////////////
        //////////////////////// Histograms ////////////////////////
        ////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////
        // GenLevel event weights, before and after each correction
        ////////////////////////////////////////////////////////////
        TH1D* hGenEvtWeight;

        ////////////////////////////////////////////////////////////
        // Before event selection
        ////////////////////////////////////////////////////////////

        // GenLevel Object histograms
        TH1D* hGen_Muon_pT;
        TH1D* hGen_Muon_phi;
        TH1D* hGen_Muon_eta;

        TH1D* hGen_Nu_pT;
        TH1D* hGen_Nu_phi;
        TH1D* hGen_Nu_eta;

        TH1D* hGen_MET_phi;
        TH1D* hGen_MET_pT;

        // For GenLevel W decaying to muon and neutrino
        TH1D* hGen_WToMuNu_pT;
        TH1D* hGen_WToMuNu_eta;
        TH1D* hGen_WToMuNu_phi;
        TH1D* hGen_WToMuNu_mass;
        TH1D* hGen_WToMuNu_MT;

        // For GenLevel inclusive decaying W
        TH1D* hGen_W_pT;
        TH1D* hGen_W_eta;
        TH1D* hGen_W_phi;
        TH1D* hGen_W_mass;
        TH1D* hGen_W_MT;

        // For LHE HT
        TH1D* hLHE_HT;

        // Object histograms
        TH1D* hMuon_pT;
        TH1D* hMuon_phi;
        TH1D* hMuon_eta;
        TH1D* hMuon_mass;

        TH1D* hMET_phi;
        TH1D* hMET_pT;
        TH1D* hMET_sumET;

        TH1D* hPFMET_phi;
        TH1D* hPFMET_pT;
        TH1D* hPFMET_sumET;

        TH1D* hPFMET_corr_phi;
        TH1D* hPFMET_corr_pT;
        TH1D* hPFMET_corr_sumET;

        // Balance between muon and MET
        TH1D* hPt_Mu_over_MET;

        // Reconstructed W histograms
        TH1D* hDeltaPhi_Mu_MET;
        TH1D* hW_MT;

        TH1D* hDeltaPhi_Mu_PFMET;
        TH1D* hW_MT_PFMET;

        TH1D* hDeltaPhi_Mu_PFMET_corr;
        TH1D* hW_MT_PFMET_corr;

        // For NPV, NPU, NTrueInt before event selection
        // For NTrueInt -> Only present in MC
        TH1D* hNPV;
        TH1D* hNPU;
        TH1D* hNTrueInt;

        ////////////////////////////////////////////////////////////
        // After event selection (no W mass cut)
        ////////////////////////////////////////////////////////////

        // GenLevel Object histograms
        TH1D* hGen_Muon_pT_after;
        TH1D* hGen_Muon_phi_after;
        TH1D* hGen_Muon_eta_after;

        TH1D* hGen_Nu_pT_after;
        TH1D* hGen_Nu_phi_after;
        TH1D* hGen_Nu_eta_after;

        TH1D* hGen_MET_phi_after;
        TH1D* hGen_MET_pT_after;

        // For GenLevel W decaying to muon and neutrino
        TH1D* hGen_WToMuNu_pT_after;
        TH1D* hGen_WToMuNu_eta_after;
        TH1D* hGen_WToMuNu_phi_after;
        TH1D* hGen_WToMuNu_mass_after;
        TH1D* hGen_WToMuNu_MT_after;

        // For GenLevel inclusive decaying W
        // This cannot exist because after event selection, muon filtering is already done
        TH1D* hGen_W_pT_after;
        TH1D* hGen_W_eta_after;
        TH1D* hGen_W_phi_after;
        TH1D* hGen_W_mass_after;
        TH1D* hGen_W_MT_after;

        // For LHE HT
        TH1D* hLHE_HT_after;

        // Object histograms
        TH1D* hMuon_pT_after;
        TH1D* hMuon_phi_after;
        TH1D* hMuon_eta_after;
        TH1D* hMuon_mass_after;

        TH1D* hMET_phi_after;
        TH1D* hMET_pT_after;
        TH1D* hMET_sumET_after;

        TH1D* hPFMET_phi_after;
        TH1D* hPFMET_pT_after;
        TH1D* hPFMET_sumET_after;

        TH1D* hPFMET_corr_phi_after;
        TH1D* hPFMET_corr_pT_after;
        TH1D* hPFMET_corr_sumET_after;

        // Balance between muon and MET
        TH1D* hPt_Mu_over_MET_after;

        // Reconstructed W histograms
        TH1D* hDeltaPhi_Mu_MET_after;
        TH1D* hW_MT_after;

        TH1D* hDeltaPhi_Mu_PFMET_after;
        TH1D* hW_MT_PFMET_after;

        TH1D* hDeltaPhi_Mu_PFMET_corr_after;
        TH1D* hW_MT_PFMET_corr_after;

        // For NPV, NPU, NTrueInt before event selection
        // For NTrueInt -> Only present in MC
        TH1D* hNPV_after;
        TH1D* hNPU_after;
        TH1D* hNTrueInt_after;

};

#endif
