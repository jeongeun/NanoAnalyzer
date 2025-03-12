#include "PU.h"
#include <fstream>
#include "correction.h"

using namespace std;

PU::~PU() {
    Clear();
}

void PU::Clear() {
    delete hDataPUHist_nominal;
    delete hDataPUHist_up     ;
    delete hDataPUHist_down   ;
    delete hMCPUHist;
    delete hPUWeights        ;
    delete hPUWeights_up     ;
    delete hPUWeights_down   ;

    fDataPUFile_nominal->Close();
    fDataPUFile_up     ->Close();
    fDataPUFile_down   ->Close();
    fMCPUFile->Close();
}

void PU::Init() {
    if (isinit) {
        cerr << "PU::Init() - PU is already initialized" << endl;
        return;
    };

    // cms-nanoAOD : jsonpog-integration, Get PU weight from POG LUM json file
    // https://gitlab.cern.ch/cms-nanoAOD/jsonpog-integration/-/tree/master/POG/LUM?ref_type=heads
    string subDirName = "";
    string Pileup_key = "";
    string fileNameLUM = "";

    if (DataEra == "2016preVFP"){   
        subDirName = "2016preVFP_UL";     Pileup_key = "Collisions16_UltraLegacy_goldenJSON";
    } 
    else if (DataEra == "2016postVFP"){  
        subDirName = "2016postVFP_UL";    Pileup_key = "Collisions16_UltraLegacy_goldenJSON";
    } 
    else if (DataEra == "2017"){
        subDirName = "2017_UL";           Pileup_key = "Collisions17_UltraLegacy_goldenJSON";
    } 
    else if (DataEra == "2018"){
        subDirName = "2018_UL";           Pileup_key = "Collisions18_UltraLegacy_goldenJSON";
    } 
    else if (DataEra == "2022preEE"){    
        subDirName = "2022_Summer22";     Pileup_key = "Collisions2022_355100_357900_eraBCD_GoldenJson";
    } 
    else if (DataEra == "2022postEE"){   
        subDirName = "2022_Summer22EE";   Pileup_key = "Collisions2022_359022_362760_eraEFG_GoldenJson";
    } 
    else if (DataEra == "2023preBPix"){  
        subDirName = "2022_Summer23";     Pileup_key = "Collisions2023_366403_369802_eraBC_GoldenJson";
    } 
    else if (DataEra == "2023postBPix"){ 
        subDirName = "2022_Summer23BPix"; Pileup_key = "Collisions2023_369803_370790_eraD_GoldenJson";
    } 
    else {
        cerr << "[ERROR] - Unknown DataEra: " << DataEra << endl;
        return;
    }

    const string dirName = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/";
    this->fileNameLUM = dirName + "LUM/" + subDirName + "/puWeights.json.gz";

   ///* ---------Get PU weight By using pileup/roots/ files
    string DataPU_nominal = ""; 
    string DataPU_up      = "";
    string DataPU_down    = "";

    // 2016preVFP and 2016postVFP use same PU scenario
    if (DataEra.find("2016") != string::npos)
        DataEra = "2016";
    if (DataEra == "2016"){
        DataPU_nominal = "PileupHistogram-goldenJSON-13tev-2016-69200ub-99bins.root";
        DataPU_up      = "PileupHistogram-goldenJSON-13tev-2016-72400ub-99bins.root";
        DataPU_down    = "PileupHistogram-goldenJSON-13tev-2016-66000ub-99bins.root";
    }else if (DataEra == "2017"){
        DataPU_nominal = "PileupHistogram-goldenJSON-13tev-2017-69200ub-99bins.root";
        DataPU_up      = "PileupHistogram-goldenJSON-13tev-2017-72400ub-99bins.root";
        DataPU_down    = "PileupHistogram-goldenJSON-13tev-2017-66000ub-99bins.root";
    }else if (DataEra == "2018"){
        DataPU_nominal = "PileupHistogram-goldenJSON-13tev-2018-69200ub-99bins.root";
        DataPU_up      = "PileupHistogram-goldenJSON-13tev-2018-72400ub-99bins.root";
        DataPU_down    = "PileupHistogram-goldenJSON-13tev-2018-66000ub-99bins.root";
    }else if (DataEra == "2022preEE"){
        DataPU_nominal = "pileupHistogram-Cert_Collisions2022_355100_357900_eraBCD_GoldenJson-13p6TeV-69200ub-99bins.root";
        DataPU_up      = "pileupHistogram-Cert_Collisions2022_355100_357900_eraBCD_GoldenJson-13p6TeV-72400ub-99bins.root";
        DataPU_down    = "pileupHistogram-Cert_Collisions2022_355100_357900_eraBCD_GoldenJson-13p6TeV-66000ub-99bins.root";
    }else {
        cerr << "[ERROR] Unknown DataEra: " << DataEra << endl;
        return;
    }

    auto check_file = [](const string& filename) {
        ifstream file(filename);
        return file.good();
    };
    string base_path = "../pileup/roots/";
    string mc_file   = base_path + "pileup_" + DataEra + "_MC_99bins.root";
    if (DataEra.find("201") != string::npos) {mc_file   = base_path + "pileup_" + DataEra + "_UL_MC_99bins.root";}
    if (!check_file(base_path + DataPU_nominal) || !check_file(base_path + DataPU_up) ||
        !check_file(base_path + DataPU_down) || !check_file(mc_file)) {
        cerr << "[ERROR] One or more PU files are missing!" << endl;
        return;
    }

    fDataPUFile_nominal = TFile::Open( (base_path + DataPU_nominal).c_str() );
    fDataPUFile_up      = TFile::Open( (base_path + DataPU_up).c_str() );
    fDataPUFile_down    = TFile::Open( (base_path + DataPU_down).c_str() );
    fMCPUFile = TFile::Open( (mc_file).c_str() );

    if (!fDataPUFile_nominal || !fDataPUFile_up || !fDataPUFile_down || !fMCPUFile) {
        cerr << "[ERROR] Failed to open one or more PU files!" << endl;
        return;
    }

    hDataPUHist_nominal = (TH1D*)fDataPUFile_nominal->Get("pileup");
    hDataPUHist_up      = (TH1D*)fDataPUFile_up     ->Get("pileup");
    hDataPUHist_down    = (TH1D*)fDataPUFile_down   ->Get("pileup");
    hMCPUHist = (TH1D*)fMCPUFile->Get("pileup");

    hPUWeights         = static_cast<TH1D*>(hDataPUHist_nominal->Clone());
    hPUWeights_up      = static_cast<TH1D*>(hDataPUHist_up     ->Clone());
    hPUWeights_down    = static_cast<TH1D*>(hDataPUHist_down   ->Clone());

    hPUWeights        ->Scale(1. / hDataPUHist_nominal->Integral()); // Normalize data PU hist
    hPUWeights_up     ->Scale(1. / hDataPUHist_up     ->Integral()); 
    hPUWeights_down   ->Scale(1. / hDataPUHist_down   ->Integral()); 
    
    hPUWeights        ->Divide(hMCPUHist); // Divide data PU hist by MC PU hist
    hPUWeights_up     ->Divide(hMCPUHist); 
    hPUWeights_down   ->Divide(hMCPUHist); 
//------ */

    PrintInitInfo();
    isinit = true;
}

void PU::PrintInitInfo() {
    cout << "----------------------------------------------------" << endl;
    cout << "[Info] PU::PrintInitInfo() - Pileup is initialized"   << endl;
    cout << "[Info] PU::PrintInitInfo() - Era: "                   << DataEra << endl;
    cout << "[Info] PU::PrintInitInfo() - fileNameLUM: "           << this->fileNameLUM << endl;
    cout << "----------------------------------------------------" << endl;
/*-------
    cout << "[Info] PU::PrintInitInfo() - Data PU (nominal)file: " << fDataPUFile_nominal->GetName() << endl;
    cout << "[Info] PU::PrintInitInfo() - Data PU (up)file: "      << fDataPUFile_up->GetName() << endl;
    cout << "[Info] PU::PrintInitInfo() - Data PU (down)file: "    << fDataPUFile_down->GetName() << endl;
    cout << "[Info] PU::PrintInitInfo() - MC PU file: "            << fMCPUFile->GetName() << endl;
-------*/
}

Double_t PU::Eval_PUWeight(string DataEra, Float_t nTrueInt, string sys_weight){
    if (!isinit) {
        cerr << "PU::Eval_PUWeight() - PU is not initialized" << endl;
        return 0;
    }
    Double_t weight = 1.0;
    unique_ptr<correction::CorrectionSet> pileupSF;
    pileupSF = correction::CorrectionSet::from_file(this->fileNameLUM);
    if (pileupSF != nullptr) {
        try {
            weight *= (*pileupSF->begin()).second->evaluate({nTrueInt, sys_weight});
        } catch (const exception& e) {
            cerr << "[ERROR] PU::Eval_PUWeight() - Error during evaluation" << endl;
            cerr << "[EXCEPTION] " << e.what() << endl;
            return 0;
        }
    }
    return weight;
}

///*-------    
Double_t PU::GetPUWeight(Float_t nTrueInt) {
    if (!isinit) {
        cerr << "PU::GetPUWeight() - PU is not initialized" << endl;
        return 0;
    }

    // Check if nTrueInt is within valid range
    Float_t fPUWeight_min = hPUWeights->GetBinLowEdge(1);
    Float_t fPUWeight_max = hPUWeights->GetBinLowEdge(hPUWeights->GetNbinsX() + 1);

    Float_t fPUWeight_up_min = hPUWeights_up->GetBinLowEdge(1);
    Float_t fPUWeight_up_max = hPUWeights_up->GetBinLowEdge(hPUWeights_up->GetNbinsX() + 1);

    Float_t fPUWeight_down_min = hPUWeights_down->GetBinLowEdge(1);
    Float_t fPUWeight_down_max = hPUWeights_down->GetBinLowEdge(hPUWeights_down->GetNbinsX() + 1);

    if (nTrueInt < fPUWeight_min || nTrueInt >= fPUWeight_max) {
        cerr << "PU::GetPUWeight() - True interaction number out of range" << endl;
        return 0;
    }

    Int_t bin      = hPUWeights->GetXaxis()->FindBin(nTrueInt);
    Int_t bin_up   = hPUWeights_up->GetXaxis()->FindBin(nTrueInt);
    Int_t bin_down = hPUWeights_down->GetXaxis()->FindBin(nTrueInt);

    return hPUWeights->GetBinContent(bin);
}
