#include "NanoAnalyzer.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <filesystem>

using namespace std;

int main(int argc, char* argv[]) {
    constexpr int REQUIRED_ARGS = 11; 

    if (argc != REQUIRED_ARGS) {
        cerr << "---------------------------------------------------------\n";
        cerr << "[Error] Incorrect number of arguments!\n";
        cerr << "[Usage] ./NanoAnalysis <InputFileList> <Era> <ProcessName> <IsMC> "
                     "<DoPUCorrection> <DoRocco> <DoIDSF> <DoIsoSF> <DoTrigSF> <OutputFile>\n";
        cerr << "---------------------------------------------------------\n";
        return EXIT_FAILURE;
    }
    // translate string to boolean
    auto to_bool = [](const string& str) {
        try {
            return stoi(str) != 0;
        } catch (const invalid_argument&) {
            cerr << "[Error] Invalid boolean argument: " << str << endl;
            return false;
        }
    };

    string inputlist = argv[1];
    string DataEra = argv[2];
    string process = argv[3];
    bool isMC = to_bool(argv[4]);
    bool bDoPUCorrection = to_bool(argv[5]); //bool bDoL1PreFiringCorrection = to_bool(argv[6]); //for Run2UL
    bool bDoRocco = to_bool(argv[6]);
    bool bDoIDSF = to_bool(argv[7]);
    bool bDoIsoSF = to_bool(argv[8]);
    bool bDoTrigSF = to_bool(argv[9]);
    string outname = argv[10];

    if (isMC == 0 ) { bDoPUCorrection = 0;  bDoRocco = 0; bDoIDSF = 0; bDoIsoSF = 0; bDoTrigSF = 0; }

    array<string, 3> SF_Histograms = { //root file
        "NUM_HighPtID_DEN_TrackerMuons_abseta_pt",
        "NUM_LooseRelTkIso_DEN_HighPtIDandIPCut_abseta_pt",
        "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose_abseta_pt" //need to be updated
    };

    if (DataEra.find("2016") != string::npos) {
        SF_Histograms[2] = "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt";
    } else if (DataEra == "2017") {
        SF_Histograms[2] = "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt";
    }
    unordered_map<string, string> roccorFiles = {
        {"2016APV", "../RoccoR/RoccoR2016aUL.txt"},
        {"2016", "../RoccoR/RoccoR2016bUL.txt"},
        {"2017", "../RoccoR/RoccoR2017UL.txt"},
        {"2018", "../RoccoR/RoccoR2018UL.txt"},
        {"2022preEE", "/d0/scratch/jelee/workspace/nanoaod-earlyRun3/WP241121/teststand/NanoAnalyzer/RoccoR/RoccoR2018UL.txt"} //[FIX ME] for the test, we used just 2018 txt
    };

    string roccorFileName = roccorFiles.count(DataEra) ? roccorFiles[DataEra] : "";
    if (roccorFileName.empty() || !filesystem::exists(roccorFileName)) {
        cerr << "[Warning] Rochester correction file not found: " << roccorFileName << endl;
        cerr << "[Warning] Rochester corrections will not be applied.\n";
        bDoRocco = false;
    }
    cout << "---------------------------------------------------------\n";
    cout << "[Info] Main.cc - Starting NanoAnalysis\n";
    cout << "[Info] Input file list : " << inputlist << "\n"
              << "[Info] Data Era : " << DataEra << "\n"
              << "[Info] Process Name : " << process << "\n"
              << "[Info] isMC : " << isMC << "\n"
              << "[Info] DoPUreweighting : " << bDoPUCorrection << "\n"
              << "[Info] DoRochesterCorrection : " << bDoRocco << "\n"
              << "[Info] DoIDSF : " << bDoIDSF << "\n"
              << "[Info] DoIsoSF : " << bDoIsoSF << "\n"
              << "[Info] DoTrigSF : " << bDoTrigSF << "\n"
              << "[Info] Output file name: " << outname << "\n";
    cout << "---------------------------------------------------------\n";

    auto Analyzer = make_unique<NanoAnalyzer>(
        inputlist, process, DataEra, 
        SF_Histograms[0], SF_Histograms[1], SF_Histograms[2], 
        roccorFileName, isMC, bDoPUCorrection, //bDoL1PreFiringCorrection, 
        bDoRocco, bDoIDSF, bDoIsoSF, bDoTrigSF
    );

    Analyzer->Init();

    Analyzer->Analyze();

    TFile* out = TFile::Open(outname.c_str(), "RECREATE");
    if (!out || out->IsZombie()) {
        cerr << "[ERROR] Failed to recreate output rootfile: " << outname << endl;
        return EXIT_FAILURE;
    }
    Analyzer->WriteHistograms(out);
    out->Close();

    cout << "---------------------------------------------------------\n";
    cout << "[SUCCESS] NanoAnalysis completed successfully!!\n";
    cout << "[SUCCESS] Final outputs created in " << outname << "\n";
    cout << "---------------------------------------------------------\n";

    return EXIT_SUCCESS;
}
