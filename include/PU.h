#ifndef PU_h
#define PU_h

#include "TFile.h"
#include "TH1.h"
#include "correction.h"

#include <string>
#include <vector>
#include <iostream>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class PU {
    private :
        string DataEra;
        Bool_t isinit = false;

        const string dirName;
        string subDirName;
        string corrNameLUM;
        string fileNameLUM;

        string DataPU_nominal;
        string DataPU_up;
        string DataPU_down;

        TFile* fDataPUFile_nominal = nullptr;
        TFile* fDataPUFile_up      = nullptr;
        TFile* fDataPUFile_down    = nullptr;
        TFile* fMCPUFile = nullptr;

        TH1D* hDataPUHist_nominal = nullptr;
        TH1D* hDataPUHist_up      = nullptr;
        TH1D* hDataPUHist_down    = nullptr;
        TH1D* hMCPUHist = nullptr;
        TH1D* hPUWeights         = nullptr;
        TH1D* hPUWeights_up      = nullptr;
        TH1D* hPUWeights_down    = nullptr;


    public :
        PU(const string& era): DataEra(era)
        {};
        virtual ~PU();

	unique_ptr<correction::CorrectionSet> pileupSF;
        void Init();
        void Clear();
        void PrintInitInfo();

        Double_t Eval_PUWeight(string DataEra, Float_t nTrueInt, string sys_weight);//using json

        Double_t GetPUWeight(Float_t nTrueInt); //using pu root files
};

#endif
