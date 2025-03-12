#ifndef GenPart_h
#define GenPart_h

// DYanalysis classes
#include "NanoDataLoader.h"
// ROOT classes
#include "TLorentzVector.h"
// C++ classes
#include <iostream>
#include <vector>
#include <string>
#include <bitset>

// Class of a single gen particle
class GenPartHolder {
    private :
        TLorentzVector GenPartVec;
        Int_t iGenPartIdx = -1;
        Int_t iGenPartCharge = -1;
        Int_t iGenPartPDGID = -1;
        Int_t iGenPartStatus = -1;
        Int_t iGenPartStatusFlags = -1;
        Int_t iGenPartMotherIdx = -1;
        Int_t iGenPartMotherPDGID = -1;
        Int_t iGenPartMotherStatus = -1;

    public :
        GenPartHolder(const TLorentzVector& vec, Int_t idx, Int_t charge, Int_t pdgId, Int_t status, Int_t statusFlags)
            : GenPartVec(vec), iGenPartIdx(idx), iGenPartCharge(charge), iGenPartPDGID(pdgId), iGenPartStatus(status), iGenPartStatusFlags(statusFlags)
        {};
        ~GenPartHolder() {};

        // Setters
        void SetGenPartIdx(Int_t idx) { iGenPartIdx = idx; }
        void SetGenPartCharge(Int_t charge) { iGenPartCharge = charge; }
        void SetGenPartPDGID(Int_t pdgId) { iGenPartPDGID = pdgId; }
        void SetGenPartStatus(Int_t status) { iGenPartStatus = status; }
        void SetGenPartStatusFlags(Int_t statusFlags) { iGenPartStatusFlags = statusFlags; }
        void SetGenPartMotherIdx(Int_t motherIdx) { iGenPartMotherIdx = motherIdx; }
        void SetGenPartMotherPDGID(Int_t motherPDGID) { iGenPartMotherPDGID = motherPDGID; }
        void SetGenPartMotherStatus(Int_t motherStatus) { iGenPartMotherStatus = motherStatus; }

        // Getters
        Double_t Pt() { return GenPartVec.Pt(); }
        Double_t Eta() { return GenPartVec.Eta(); }
        Double_t Phi() { return GenPartVec.Phi(); }
        Double_t M() { return GenPartVec.M(); }
        Int_t Charge() { return iGenPartCharge; }

        // Getters for GenPartHolder
        const TLorentzVector& GetGenPartVec() const { return GenPartVec; }
        Int_t GetGenPartIdx() { return iGenPartIdx; }
        Int_t GetGenPartPDGID() { return iGenPartPDGID; }
        Int_t GetGenPartStatus() { return iGenPartStatus; }
        Int_t GetGenPartStatusFlags() { return iGenPartStatusFlags; }
        Int_t GetGenPartMotherIdx() { return iGenPartMotherIdx; }
        Int_t GetGenPartMotherPDGID() { return iGenPartMotherPDGID; }
        Int_t GetGenPartMotherStatus() { return iGenPartMotherStatus; }

        void PrintGenPartInfo();
        void PrintGenPartStatusFlags();

        // Getters for process flags
        Bool_t IsPrompt();
        Bool_t IsHardProcess();
        Bool_t IsFromHardProcess();
        Bool_t IsTauDecayProduct();
        Bool_t IsPromptTauDecayProduct();
        Bool_t IsDirectTauDecayProduct();
        Bool_t IsDirectPromptTauDecayProduct();

        // PDGID checkers
        // Elec : 11
        // Electron Neutrino : 14
        // Muon : 13
        // Muon Neutrino : 12
        // Tau : 15
        // Tau Neutrino : 16
        // Neutrino is also a lepton, but not included in the checkers below
        Bool_t IsLepton() { return ( std::abs(iGenPartPDGID) == 11 || std::abs(iGenPartPDGID) == 13 || std::abs(iGenPartPDGID) == 15 ); }
        Bool_t IsNeutrino() { return ( std::abs(iGenPartPDGID) == 12 || std::abs(iGenPartPDGID) == 14 || std::abs(iGenPartPDGID) == 16 ); }
        Bool_t IsElectron() { return ( std::abs(iGenPartPDGID) == 11 ); }
        Bool_t IsMuon() { return ( std::abs(iGenPartPDGID) == 13 ); }
        Bool_t IsTau() { return ( std::abs(iGenPartPDGID) == 15 ); }
        Bool_t IsElectronNeutrino() { return ( std::abs(iGenPartPDGID) == 12 ); }
        Bool_t IsMuonNeutrino() { return ( std::abs(iGenPartPDGID) == 14 ); }
        Bool_t IsTauNeutrino() { return ( std::abs(iGenPartPDGID) == 16 ); }
};

// Class of all gen particles in an event
class GenParts {
    private :
        std::vector<GenPartHolder> vGenPartVec;
        NanoDataLoader* cData;
        // Flags for the class
        Int_t iFoundW = 0; // # of reconstructed Gen-lv W boson (l+nu)
        Int_t iFoundLepton = 0; // # of Gen-lv lepton
        Int_t iFoundNeutrino = 0; // # of Gen-lv neutrino
        Int_t iFoundMuon = 0; // # of Gen-lv muon
        Int_t iFoundMuonNeutrino = 0; // # of Gen-lv muon neutrino
        Int_t iIsWToMuNu = 0; // # of reconstructed Gen-lv W boson (mu+nu)
        Int_t iFoundTau = 0; // To find W to tau+nu to muon+nu+nu+nu
        Int_t iFoundTauNeutrino = 0; // To find W to tau+nu to muon+nu+nu+nu
        // For W->tau+nu channel only
        Int_t iFoundMuonFromTauDecay = 0; // find muon from tau decay
        Int_t iFoundMuonNeutrinoFromTauDecay = 0; // find muon neutrino from tau decay
        
        Bool_t isinit = false;
        // Finding Gen-lv muon and neutrinos
        TLorentzVector GenW;
        TLorentzVector LeptonFromW;
        TLorentzVector NeutrinoFromW;

    public :
        GenParts(NanoDataLoader* data): cData(data) {};
        ~GenParts() {};
        void Init();
        void Reset() {
            vGenPartVec.clear();
            isinit = false;
            iFoundW = 0;
            iFoundLepton = 0;
            iFoundNeutrino = 0;
            iFoundMuon = 0;
            iFoundMuonNeutrino = 0;
            iFoundTau = 0;
            iFoundTauNeutrino = 0;
            iFoundMuonFromTauDecay = 0;
            iFoundMuonNeutrinoFromTauDecay = 0;
            GenW = TLorentzVector();
            LeptonFromW = TLorentzVector();
            NeutrinoFromW = TLorentzVector();

            GenW = TLorentzVector();
            LeptonFromW = TLorentzVector(); 
            NeutrinoFromW = TLorentzVector();
        };
        void PrintGenPartChain();

        // Getters for GenPartHolder
        std::vector<GenPartHolder>& GetGenParts();
        std::vector<GenPartHolder> GetGenMuons();
        std::vector<GenPartHolder> GetGenElectrons();
        std::vector<GenPartHolder> GetGenTaus();
        std::vector<GenPartHolder> GetGenNeutrinos();

        // Getters
        UInt_t GetNGenParts() { return **(cData->nGenPart); }

        Int_t FoundW() {return iFoundW;}
        Int_t FoundLepton() {return iFoundLepton;}
        Int_t FoundNeutrino() {return iFoundNeutrino;}
        Int_t FoundMuon() {return iFoundMuon;}
        Int_t FoundMuonNeutrino() {return iFoundMuonNeutrino;}
        Int_t FoundTau() {return iFoundTau;}
        Int_t FoundTauNeutrino() {return iFoundTauNeutrino;}
        Int_t FoundMuonFromTauDecay() {return iFoundMuonFromTauDecay;}
        Int_t FoundMuonNeutrinoFromTauDecay() {return iFoundMuonNeutrinoFromTauDecay;}


        // Getters for Gen-lv muon and neutrinos
        const TLorentzVector& GetGenW() {return GenW;}
        const TLorentzVector& GetLeptonFromW() {return LeptonFromW;}
        const TLorentzVector& GetNeutrinoFromW() {return NeutrinoFromW;}

        // Gen-lv patching and muon filtering
        Bool_t PassMuonFiltering();
};


#endif
