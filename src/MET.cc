#include "MET.h"

void MET::Init() {
    if (isinit) {
        std::cerr << "[Warning] MET::Init() - MET is already initialized" << std::endl;
        return;
    }

    fMET_pt = **(cData->MET_pt);
    fMET_phi = **(cData->MET_phi);
    fMET_sumEt = **(cData->MET_sumEt);

    fPuppiMET_pt = **(cData->PuppiMET_pt);
    fPuppiMET_phi = **(cData->PuppiMET_phi);
    fPuppiMET_sumEt = **(cData->PuppiMET_sumEt);

    isinit = true;
}

// PF MET xy-Shift Correction
// Use offlineSlimmedPrimaryVertices as input npv (ref: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETRun2Corrections#xy_Shift_Correction_MET_phi_modu)
// https://github.com/cms-sw/cmssw/blob/6177d0bc79e19c4308339bbd745bad4cb989237c/PhysicsTools/NanoAOD/plugins/VertexTableProducer.cc
// https://github.com/cms-sw/cmssw/blob/6177d0bc79e19c4308339bbd745bad4cb989237c/PhysicsTools/NanoAOD/python/vertices_cff.py 
std::pair<Double_t, Double_t> MET::GetPFMETXYCorr(std::string processName, std::string era, Bool_t isMC, Int_t n_PV) {

    if (!isinit) {
        std::cerr << "[Warning] MET::GetPFMETXYCorr() - MET is not initialized" << std::endl;
        return std::make_pair(-999, -999);
    }
    
    Int_t NPV = n_PV; 
    if (NPV > 100) NPV = 100; 

    Double_t METxcorr = 0.;
    Double_t METycorr = 0.;

    // Data 2018 (A~D)
    if ( (!isMC) && (processName == "SingleMuon_Run2018A") ) { METxcorr = -(0.263733*NPV +-1.91115); METycorr = -(0.0431304*NPV +-0.112043);}
    if ( (!isMC) && (processName == "SingleMuon_Run2018B") ) { METxcorr = -(0.400466*NPV +-3.05914); METycorr = -(0.146125*NPV +-0.533233);}
    if ( (!isMC) && (processName == "SingleMuon_Run2018C") ) { METxcorr = -(0.430911*NPV +-1.42865); METycorr = -(0.0620083*NPV +-1.46021);}
    if ( (!isMC) && (processName == "SingleMuon_Run2018D") ) { METxcorr = -(0.457327*NPV +-1.56856); METycorr = -(0.0684071*NPV +-0.928372);}
    // MC
    if (isMC && era == "2016APV") { METxcorr = -(-0.188743*NPV +0.136539);  METycorr = -(0.0127927*NPV +0.117747); }
    if (isMC && era == "2016")    { METxcorr = -(-0.153497*NPV +-0.231751); METycorr = -(0.00731978*NPV +0.243323); }
    if (isMC && era == "2017")    { METxcorr = -(-0.300155*NPV +1.90608);   METycorr = -(0.300213*NPV +-2.02232); }
    if (isMC && era == "2018")    { METxcorr = -(-0.183518*NPV +0.546754);  METycorr = -(0.192263*NPV +-0.42121); }
    
    Double_t CorrectedMET_x = fMET_pt *std::cos(fMET_phi) + METxcorr;
    Double_t CorrectedMET_y = fMET_pt *std::sin(fMET_phi) + METycorr;

    Double_t CorrectedMET = std::sqrt(CorrectedMET_x*CorrectedMET_x + CorrectedMET_y*CorrectedMET_y);
    Double_t CorrectedMETPhi;

    if     (CorrectedMET_x==0 && CorrectedMET_y>0) CorrectedMETPhi = TMath::Pi();
    else if(CorrectedMET_x==0 && CorrectedMET_y<0 )CorrectedMETPhi = -TMath::Pi();
    else if(CorrectedMET_x >0)                     CorrectedMETPhi = TMath::ATan(CorrectedMET_y/CorrectedMET_x);
    else if(CorrectedMET_x <0 && CorrectedMET_y>0) CorrectedMETPhi = TMath::ATan(CorrectedMET_y/CorrectedMET_x) + TMath::Pi();
    else if(CorrectedMET_x <0 && CorrectedMET_y<0) CorrectedMETPhi = TMath::ATan(CorrectedMET_y/CorrectedMET_x) - TMath::Pi();
    else CorrectedMETPhi = 0;

    return std::make_pair(CorrectedMET, CorrectedMETPhi);
}
