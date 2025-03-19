#include "NanoAnalyzer.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Starting Event loop /////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NanoAnalyzer::Analyze() {
    // Check if NanoAnalyzer is initialized
    if (!isinit) {
        throw runtime_error("[Runtime Error] NanoAnalyzer::Analyze() - NanoAnalyzer is not initialized");
    }

    cout << "[Debug] ReadNextEntry() is True?  : "  << cData->ReadNextEntry() << endl;

    // Declare object classes
    Muons* muons = new Muons(cData);
    MET* met = new MET(cData);
    GenParts* genparticles = nullptr;
    if (isMC) {
        genparticles = new GenParts(cData);
    }


    cout << "[Info] NanoAnalyzer::Analyze() - Start event loop" << endl;
    int ievent = 0;
    while (cData->ReadNextEntry()) {
        // Print percent and set current event number
        if (ievent % 1000 == 0) PrintProgress(ievent);
        ievent++;

        // Reset & Init object classes
        muons->Reset();
        met->Reset();
        muons->Init();
        met->Init();
        if (isMC) {
            genparticles->Reset();
            genparticles->Init();
        }
        // Get corrected PFMET
        pair<Double_t, Double_t> correctedPFMET = met->GetPFMETXYCorr(process, DataEra, isMC, **(cData->NPV));
        Double_t dPFMET_corr = correctedPFMET.first;
        Double_t dPFMET_corr_phi = correctedPFMET.second;

        // Set event weight (For data, event weight is always 1.0)
        Double_t eventWeight = 1.0;
        Bool_t hasNegativeWeight = false;
        if (isMC) {
            eventWeight = **(cData->GenWeight) < 0 ? -1.0 : 1.0;
            if (eventWeight < 0) { 
                hasNegativeWeight = true;
                cout << "[Warning] !! Negative GenWeight is Found " << endl;
            }
        }
        if (hasNegativeWeight) continue;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// STEP 0. Apply Correction ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////
        /////// Pileup Corrections ////////
        ///////////////////////////////////

        if (isMC && bDoPUCorrection) {
            // Get PU weight
            Float_t nTrueInt = **(cData->Pileup_nTrueInt);
            eventWeight *= cPU->Eval_PUWeight(DataEra, nTrueInt, "nominal"); //use json
            //cout << "[DEBUG] PU Reweighting Check => eventWeight (isMC && bDoPUCorrection) " << eventWeight << endl;
            //eventWeight *= cPU->GetPUWeight(nTrueInt); //if you use hist root instead json
        }

        //////////////////////////////////////
        //// Muon Rochester Corrections //////
        //////////////////////////////////////

        if (bDoRocco) {
            // Loop over muons
            vector<MuonHolder>& muonCollection = muons->GetMuons();
            for (MuonHolder& singleMuon : muonCollection) {
                // Rocco for MC
                if (isMC) {
                    // Gen - Reco muon matching using dR
                    Double_t min_dR = 999.;
                    Bool_t matchedToGenMuon = false;
                    Int_t matchedGenMuonIdx = -1;
                    // Get gen muons
                    vector<GenPartHolder> genMuonCollection = genparticles->GetGenMuons();
                    for (Int_t idx = 0; idx < genMuonCollection.size(); idx++) {
                        GenPartHolder& singleGenMuon = genMuonCollection[idx];
                        // Get dR between gen muon and reco muon
                        Double_t dR = (singleGenMuon.GetGenPartVec()).DeltaR(singleMuon.GetMuonOrgVec());
                        if (dR < min_dR && dR < 0.1) {
                            min_dR = dR;
                            matchedToGenMuon = true;
                            matchedGenMuonIdx = idx;
                        }
                    }
                    // If well matched
                    if (matchedToGenMuon) {
                        Double_t roccoSF = cRochesterCorrection->kSpreadMC(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), genMuonCollection[matchedGenMuonIdx].GetGenPartVec().Pt(), 5, 0);
                        singleMuon.SetRoccoSF(roccoSF);
                        //cout << "[DEBUG] Rochester Check MC => matched roccoSF " << roccoSF << endl; 
                    }
                    // If not matched
                    else {
                        Double_t randomSeed = gRandom->Rndm(); // Random seed btw 0 ~ 1
                        Double_t roccoSF = cRochesterCorrection->kSmearMC(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), singleMuon.GetTrackerLayers(), randomSeed, 5, 0);
                        singleMuon.SetRoccoSF(roccoSF);
                        //cout << "[DEBUG] Rochester Check MC => un-matched roccoSF " << roccoSF << endl; 
                    }
                }
                // Rocco for data
                else {
                    Double_t roccoSF = cRochesterCorrection->kScaleDT(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), 5, 0);
                    //cout << "[DEBUG] Rochester Check Data => roccoSF " << roccoSF << endl; 
                    singleMuon.SetRoccoSF(roccoSF);
                }
            }
        }

        /////////////////////////////////////////
        ////// Muon ID+ISO+HLT Corrections //////
        /////////////////////////////////////////

        // Do object selection here
        muons->DoObjSel();

        // Do Muon SF correction (to tightID and leading pT Muon)
        if (isMC) {
            if (bDoIDSF || bDoIsoSF || bDoTrigSF) {
                vector<double> muCorrSF = {1.0, 1.0, 1.0}; // {id_sf, iso_sf, hlt_sf}
                double minpt   = 52.0; // pT edge (please check min edge for your key in the json)

                // Set efficiency SF over all muons
                vector<MuonHolder> tightMuonCollection = muons->GetTightMuons();
                if (tightMuonCollection.size() > 0) {
                    MuonHolder& leadingMuon = tightMuonCollection[0]; //pick only leading muon
                    muCorrSF = cEfficiencySF->GetMuonSF(minpt, leadingMuon.Pt(), leadingMuon.Eta(), "nominal");//"systup/down

                    //muon_sf_test = cEfficiencySF->GetMuonIDSF(tightMuonCollection, "nominal"); //test for many muons in the event

                    // Get and apply SF using Root file
                    //muCorrSF = cEfficiencySF->GetEfficiency(leadingMuon.Pt(), leadingMuon.Eta());
                    //cout << "[DEBUG] LeadingMuon PT= " << leadingMuon.Pt() <<", Eta= " << leadingMuon.Eta() << endl; 
                    leadingMuon.SetEfficiencySF(muCorrSF);
                }

                if (bDoIDSF)   eventWeight *= muCorrSF[0]; // ID SF
                if (bDoIsoSF)  eventWeight *= muCorrSF[1]; // Iso SF
                if (bDoTrigSF) eventWeight *= muCorrSF[2]; // Trig SF

                //cout << "[DEBUG] Muon ID, ISO, HLT Scale factor => id " 
                //          << muCorrSF[0] << " iso " << muCorrSF[1] << " hlt " << muCorrSF[2] 
                //          << ", eventWeight " << eventWeight << endl; 
            }
        
            hNPU->Fill(**(cData->Pileup_nPU), eventWeight);
            hNTrueInt->Fill(**(cData->Pileup_nTrueInt), eventWeight);
        }

        hNPV->Fill(**(cData->NPV), eventWeight);
        /////////////////////////////////////////////////
        // Sum up event weight here (after all corrs) ///
        /////////////////////////////////////////////////
        dSumOfGenEvtWeight += eventWeight;
        cout << "[CHECKING] SumUp eventWeight after all correction dSumOfGenEvtWeight = " << dSumOfGenEvtWeight << endl; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// STEP 1. Fill Histo before Selection ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////
        ////// Apply Gen-lv patching and Gen-lv muon filtering /////
        ////////////////////////////////////////////////////////////
        if (isMC) {
            // Fill Gen Muon
            vector<GenPartHolder> genMuonCollection = genparticles->GetGenMuons();
            // Sort genMuonCollection in descending order by Pt
            if (genMuonCollection.size() > 0) {
                // vector<GenPartHolder>::iterator type
                auto maxElemIt = max_element(genMuonCollection.begin(), genMuonCollection.end(),
                    [](const GenPartHolder &a, const GenPartHolder &b) {
                        return a.GetGenPartVec().Pt() < b.GetGenPartVec().Pt();
                    }
                );
                GenPartHolder &leadingGenMuon = *maxElemIt;
                hGen_Muon_pT->Fill(leadingGenMuon.GetGenPartVec().Pt(), eventWeight);
                //cout << "[TESTING] : GetGenMuon pT = " << leadingGenMuon.GetGenPartVec().Pt() << endl;
                hGen_Muon_phi->Fill(leadingGenMuon.GetGenPartVec().Phi(), eventWeight);
                hGen_Muon_eta->Fill(leadingGenMuon.GetGenPartVec().Eta(), eventWeight);
            }

            // Fill Gen Neutrino
            vector<GenPartHolder> genNeutrinoCollection = genparticles->GetGenNeutrinos();
            // Sort genNeutrinoCollection in descending order by Pt
            if (genNeutrinoCollection.size() > 0) {
                auto maxElemIt = max_element(genNeutrinoCollection.begin(), genNeutrinoCollection.end(),
                    [](const GenPartHolder &a, const GenPartHolder &b) {
                        return a.GetGenPartVec().Pt() < b.GetGenPartVec().Pt();
                    }
                );
                GenPartHolder &leadingGenNeutrino = *maxElemIt;
                hGen_Nu_pT->Fill(leadingGenNeutrino.GetGenPartVec().Pt(), eventWeight);
                hGen_Nu_phi->Fill(leadingGenNeutrino.GetGenPartVec().Phi(), eventWeight);
                hGen_Nu_eta->Fill(leadingGenNeutrino.GetGenPartVec().Eta(), eventWeight);
            }

            // Fill Gen MET
            hGen_MET_phi->Fill(cData->GenMET_phi->At(0), eventWeight);
            hGen_MET_pT->Fill(cData->GenMET_pt->At(0), eventWeight);
        }

        ////////////////////////////////////////////////////////////
        //// Fill RECO-level histograms before event selection /////
        ////////////////////////////////////////////////////////////
        // Fill Tight muon
        if (muons->GetTightMuons().size() > 0) {
            MuonHolder& leadingMuon = muons->GetTightMuons()[0];
            TLorentzVector leadingMuonVec = leadingMuon.GetRoccoSF() == -1. ? leadingMuon.GetMuonOrgVec() : leadingMuon.GetMuonRoccoVec();
            
            hMuon_pT->Fill(leadingMuonVec.Pt(), eventWeight);
            hMuon_phi->Fill(leadingMuonVec.Phi(), eventWeight);
            hMuon_eta->Fill(leadingMuonVec.Eta(), eventWeight);
        }

        // Fill MET, Puppi, PF
        hMET_phi->Fill(met->GetPuppiMET_phi(), eventWeight);
        hMET_pT->Fill(met->GetPuppiMET_pt(), eventWeight);
        hMET_sumET->Fill(met->GetPuppiMET_sumEt(), eventWeight);

        hPFMET_phi->Fill(met->GetMET_phi(), eventWeight);
        hPFMET_pT->Fill(met->GetMET_pt(), eventWeight);
        hPFMET_sumET->Fill(met->GetMET_sumEt(), eventWeight);

        hPFMET_corr_phi->Fill(dPFMET_corr_phi, eventWeight);
        hPFMET_corr_pT->Fill(dPFMET_corr, eventWeight);

        // Fill Reconstructed value dPhi ptratio, MT
        if (muons->GetTightMuons().size() > 0) {
            MuonHolder& leadingMuon = muons->GetTightMuons()[0];
            TLorentzVector leadingMuonVec = leadingMuon.GetRoccoSF() == -1. ? leadingMuon.GetMuonOrgVec() : leadingMuon.GetMuonRoccoVec();

            Double_t deltaPhi = TVector2::Phi_mpi_pi(met->GetPuppiMET_phi() - leadingMuonVec.Phi());
            Double_t deltaPhi_PFMET = TVector2::Phi_mpi_pi(met->GetMET_phi() - leadingMuonVec.Phi());
            Double_t deltaPhi_PFMET_corr = TVector2::Phi_mpi_pi(dPFMET_corr_phi - leadingMuonVec.Phi());

            Double_t W_MT = sqrt( 2 * leadingMuonVec.Pt() * met->GetPuppiMET_pt() * (1 - cos(deltaPhi)) );
            Double_t W_MT_PFMET = sqrt( 2 * leadingMuonVec.Pt() * met->GetMET_pt() * (1 - cos(deltaPhi_PFMET)) );
            Double_t W_MT_PFMET_corr = sqrt( 2 * leadingMuonVec.Pt() * dPFMET_corr * (1 - cos(deltaPhi_PFMET_corr)) );

            hDeltaPhi_Mu_MET->Fill(abs(deltaPhi), eventWeight);
            hPt_Mu_over_MET->Fill(leadingMuonVec.Pt() / met->GetPuppiMET_pt(), eventWeight);
            hW_MT->Fill(W_MT, eventWeight);

            hDeltaPhi_Mu_PFMET->Fill(abs(deltaPhi_PFMET), eventWeight);
            hW_MT_PFMET->Fill(W_MT_PFMET, eventWeight);

            hDeltaPhi_Mu_PFMET_corr->Fill(abs(deltaPhi_PFMET_corr), eventWeight);
            hW_MT_PFMET_corr->Fill(W_MT_PFMET_corr, eventWeight);
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// STEP 2. Fill Histo after Selection ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        ///////////////////////////////////////////////////////////////////////////////////
        ////////// Event Selection: NoiseFilter, HLT, MuonID, Dilepton Veto, MT cut////////
        ///////////////////////////////////////////////////////////////////////////////////
        // #1. Noise filter
        // For 2016: Do not use Flag_ecalBadCalibFilter, Flag_BadChargedCandidateFilter
        // For 2017, 2018: Do not use Flag_BadChargedCandidateFilter
        Bool_t flag_goodVertices                       =  **(cData->Flag_goodVertices);
        Bool_t flag_globalSuperTightHalo2016Filter     =  **(cData->Flag_globalSuperTightHalo2016Filter);
        Bool_t flag_EcalDeadCellTriggerPrimitiveFilter =  **(cData->Flag_EcalDeadCellTriggerPrimitiveFilter);
        Bool_t flag_BadPFMuonFilter                    =  **(cData->Flag_BadPFMuonFilter);
        Bool_t flag_BadPFMuonDzFilter                  =  **(cData->Flag_BadPFMuonDzFilter);
        Bool_t flag_hfNoisyHitsFilter                  =  **(cData->Flag_hfNoisyHitsFilter);
        Bool_t flag_eeBadScFilter                      =  **(cData->Flag_eeBadScFilter);
        bool passed_filter = (  flag_goodVertices                      &&
                                flag_globalSuperTightHalo2016Filter    &&
                                flag_EcalDeadCellTriggerPrimitiveFilter&&
                                flag_BadPFMuonFilter                   &&
                                flag_BadPFMuonDzFilter                 &&
                                flag_hfNoisyHitsFilter                 &&
                                flag_eeBadScFilter                     
                                );
        if (DataEra.find("2016") != string::npos) {
            //passed_filter = passed_filter && **(cData->Flag_ecalBadCalibFilter);
        }
        if (!passed_filter) continue; // Skip event if noise filter failed


        // #2. Trigger (signal)
        // 2016 : IsoMu24 || IsoTkMu24, 2017 : IsoMu27, 2018 : IsoMu24
        // 2022preEE : Mu50 || CascadeMu100 || HighPtTkMu100
        Bool_t passedTrigger = false;
        /* if (DataEra == "2016APV") {
            passedTrigger = (**(cData->HLT_IsoMu24) || **(cData->HLT_IsoTkMu24));  passedTrigger = **(cData->HLT_Mu50) ;
        } else if (DataEra == "2016") {
            passedTrigger = (**(cData->HLT_IsoMu24) || **(cData->HLT_IsoTkMu24));  passedTrigger = **(cData->HLT_Mu50) ;
        } else if (DataEra == "2017") {
            passedTrigger = **(cData->HLT_IsoMu27); passedTrigger = **(cData->HLT_Mu50) ;
        } else if (DataEra == "2018") {
            passedTrigger = **(cData->HLT_IsoMu24); passedTrigger = **(cData->HLT_Mu50) ;
        } else */
        if (DataEra == "2022preEE") {
            passedTrigger = (**(cData->HLT_Mu50) || **(cData->HLT_CascadeMu100) || **(cData->HLT_HighPtTkMu100)) ;
        }
        if (!passedTrigger) continue;


        // #3. Require only single tight muon with pt > 53 GeV
        if( muons->GetTightMuons().size() != 1 ) continue;
        if( muons->GetTightMuons()[0].Pt() < 53. ) continue;

        // #4. Vetoing the event with an additional second Loose ID muon 
        if( muons->GetLooseMuons().size() > 0 ) continue;

        // #5. Calculate MT and cut MT > 120 GeV)
        // Using PUPPI MET for this cut
        MuonHolder& leadingMuon = muons->GetTightMuons()[0];
        TLorentzVector leadingMuonVec = leadingMuon.GetRoccoSF() == -1. ? leadingMuon.GetMuonOrgVec() : leadingMuon.GetMuonRoccoVec();
        Double_t deltaPhi = TVector2::Phi_mpi_pi(met->GetPuppiMET_phi() - leadingMuonVec.Phi());
        Double_t deltaPhi_PFMET = TVector2::Phi_mpi_pi(met->GetMET_phi() - leadingMuonVec.Phi());
        Double_t deltaPhi_PFMET_corr = TVector2::Phi_mpi_pi( dPFMET_corr_phi - leadingMuonVec.Phi());

        Double_t W_MT = sqrt( 2 * leadingMuonVec.Pt() * met->GetPuppiMET_pt() * (1 - cos(deltaPhi)) );
        Double_t W_MT_PFMET = sqrt( 2 * leadingMuonVec.Pt() * met->GetMET_pt() * (1 - cos(deltaPhi_PFMET)) );
        Double_t W_MT_PFMET_corr = sqrt( 2 * leadingMuonVec.Pt() * dPFMET_corr * (1 - cos(deltaPhi_PFMET_corr)) );
        
        if (W_MT < 120.) continue;

        /////////////////////////////////////////////////////
        //////// Fill histograms after event selection //////
        /////////////////////////////////////////////////////
        // Fill PU related histograms (NPU, NTrueInt only available for MC)
        hNPV_after->Fill(**(cData->NPV), eventWeight);
        
        // Fill Gen-level histo
        if (isMC) {
            // Fill PU related histograms (NPU, NTrueInt only available for MC)
            hNPU_after->Fill(**(cData->Pileup_nPU), eventWeight);
            hNTrueInt_after->Fill(**(cData->Pileup_nTrueInt), eventWeight);

            // Fill Gen Muon
            vector<GenPartHolder> genMuonCollection = genparticles->GetGenMuons();
            // Sort genMuonCollection in descending order by Pt
            if (genMuonCollection.size() > 0) {
                auto maxElemIt = max_element(genMuonCollection.begin(), genMuonCollection.end(),
                    [](const GenPartHolder &a, const GenPartHolder &b) {
                        return a.GetGenPartVec().Pt() < b.GetGenPartVec().Pt();
                    }
                );
                GenPartHolder &leadingGenMuon = *maxElemIt;
                hGen_Muon_pT_after->Fill(leadingGenMuon.GetGenPartVec().Pt(), eventWeight);
                hGen_Muon_phi_after->Fill(leadingGenMuon.GetGenPartVec().Phi(), eventWeight);
                hGen_Muon_eta_after->Fill(leadingGenMuon.GetGenPartVec().Eta(), eventWeight);
            }

            // Fill Gen Neutrino
            vector<GenPartHolder> genNeutrinoCollection = genparticles->GetGenNeutrinos();
            // Sort genNeutrinoCollection in descending order by Pt
            if (genNeutrinoCollection.size() > 0) {
                auto maxElemIt = max_element(genNeutrinoCollection.begin(), genNeutrinoCollection.end(),
                    [](const GenPartHolder &a, const GenPartHolder &b) {
                        return a.GetGenPartVec().Pt() < b.GetGenPartVec().Pt();
                    }
                );
                GenPartHolder &leadingGenNeutrino = *maxElemIt;
                hGen_Nu_pT_after->Fill(leadingGenNeutrino.GetGenPartVec().Pt(), eventWeight);
                hGen_Nu_phi_after->Fill(leadingGenNeutrino.GetGenPartVec().Phi(), eventWeight);
                hGen_Nu_eta_after->Fill(leadingGenNeutrino.GetGenPartVec().Eta(), eventWeight);
            }

            // Fill Gen MET
            hGen_MET_phi_after->Fill(cData->GenMET_phi->At(0), eventWeight);
            hGen_MET_pT_after->Fill(cData->GenMET_pt->At(0), eventWeight);
        }

        // Fill Muon (tight muon only)
        hMuon_pT_after->Fill(leadingMuonVec.Pt(), eventWeight);
        hMuon_phi_after->Fill(leadingMuonVec.Phi(), eventWeight);
        hMuon_eta_after->Fill(leadingMuonVec.Eta(), eventWeight);

        // Fill MET
        hMET_phi_after->Fill(met->GetPuppiMET_phi(), eventWeight);
        hMET_pT_after->Fill(met->GetPuppiMET_pt(), eventWeight);
        hMET_sumET_after->Fill(met->GetPuppiMET_sumEt(), eventWeight);

        hPFMET_phi_after->Fill(met->GetMET_phi(), eventWeight);
        hPFMET_pT_after->Fill(met->GetMET_pt(), eventWeight);
        hPFMET_sumET_after->Fill(met->GetMET_sumEt(), eventWeight);

        hPFMET_corr_phi_after->Fill(dPFMET_corr_phi, eventWeight);
        hPFMET_corr_pT_after->Fill(dPFMET_corr, eventWeight);

        // Balance between muon and MET
        hPt_Mu_over_MET_after->Fill(leadingMuonVec.Pt() / met->GetPuppiMET_pt(), eventWeight);

        // Fill reconstructed W MT
        // Using PUPPI MET
        hDeltaPhi_Mu_MET_after->Fill(abs(deltaPhi), eventWeight);
        hW_MT_after->Fill(W_MT, eventWeight);
        // Using PFMET
        hDeltaPhi_Mu_PFMET_after->Fill(abs(deltaPhi_PFMET), eventWeight);
        hW_MT_PFMET_after->Fill(W_MT_PFMET, eventWeight);
        // Using corrected PFMET
        hDeltaPhi_Mu_PFMET_corr_after->Fill(abs(deltaPhi_PFMET_corr), eventWeight);
        hW_MT_PFMET_corr_after->Fill(W_MT_PFMET_corr, eventWeight);
    } // End of event loop

    if(isMC){
        //////////////////////////////////////////////////////////
        ///////// Fill SumOfGenEvtWeight after event loop ////////
        //////////////////////////////////////////////////////////
        hGenEvtWeight->SetBinContent(1, dSumOfGenEvtWeight);

        cout << "[Info] NanoAnalyzer::Analyze() - End of event loop" << endl;
        cout << "[Info] NanoAnalyzer::Analyze() - Total sum of weight: " << fixed << setprecision(2) << dSumOfGenEvtWeight << endl;
    }
}

////////////////////////////////////////////////////////////
//////////////// Class initialization //////////////////////
////////////////////////////////////////////////////////////
void NanoAnalyzer::Init() {
    // Declare classes
    cData = new NanoDataLoader(process, DataEra, inputlist, isMC);
    cout << "[Info] NanoAnalyzer::Init() - cData is defined" << endl;
    // Initialize classes
    cData->Init();
    cRochesterCorrection = new RoccoR(RoccorName); // Rocco is initialized here
    cout << "[Info] NanoAnalyzer::Init() - cRochesterCorrection is defined" << endl;
    if(isMC){
        cPU = new PU(DataEra);
        cout << "[Info] NanoAnalyzer::Init() - cPU is defined" << endl;
        cEfficiencySF = new EfficiencySF(DataEra, hName_ID, hName_Iso, hName_Trig);    
        cout << "[Info] NanoAnalyzer::Init() - cEfficiencySF is defined" << endl;
        cPU->Init();
        cEfficiencySF->Init();
    }
    // Set total events
    nTotalEvents = cData->GetTotalEvents(); 
    cout << "[Info] NanoAnalyzer::Init() - Tree nTotalEvents = " << nTotalEvents << endl;

    // Declare histograms
    this->DeclareHistograms();
    cout << "[Info] NanoAnalyzer::Init() - Declar Histograms()" << endl;
    // Print initialization information
    this->PrintInitInfo();

    isinit = true;
}


// Print initialization information
void NanoAnalyzer::PrintInitInfo() {
    cout << "-------------------------------------------------------------------------" << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - NanoAnalyzer is initialized" << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Process name: " << process << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Era: " << DataEra << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Is MC: " << isMC << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Do PU correction: " << bDoPUCorrection << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Do ID SF: " << bDoIDSF << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Do Iso SF: " << bDoIsoSF << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Do Trig SF: " << bDoTrigSF << endl;
    cout << "[Info] NanoAnalyzer::PrintInitInfo() - Do rocco correction: " << bDoRocco << endl;
    cout << "-------------------------------------------------------------------------" << endl;
}

// Simple utility to print percent
void NanoAnalyzer::PrintProgress(const int currentStep) {    
    if (nTotalEvents <= 0) {
        cerr << "[ERROR] nTotalEvents is zero or negative. Cannot calculate progress." << endl;
        return;
    }

    float percent = (float)currentStep / nTotalEvents;
    int width = 70;

    cout << "[";
    int pos = width * percent;
    for (int i = 0; i < width; i++) {
        if (i < pos)
            cout << "=";
        else if (i == pos)
            cout << ">";
        else
            cout << " ";
    }
    cout << "]  " << currentStep << "/" << nTotalEvents << "  " << int(percent * 100.0) << "%\r";
    cout.flush();
}

// Declare histograms
void NanoAnalyzer::DeclareHistograms() {

    // For NPV, NPU, NTrueInt before event selection
    // For NTrueInt -> Only present in MC
    hNPV = new TH1D("hNPV", "hNPV", 100, 0, 100);
    hNPV->Sumw2();
    hNPV_after = new TH1D("hNPV_after", "hNPV_after", 100, 0, 100);
    hNPV_after->Sumw2();

    if (isMC) {
        ////////////////////////////////////////////////////////////
        // GenLevel event weights, before and after each correction
        ////////////////////////////////////////////////////////////
        hGenEvtWeight = new TH1D("hGenEvtWeight", "hGenEvtWeight", 1,0,1);
        hGenEvtWeight->Sumw2();
        ////////////////////////////////////////////////////////////
        // Before event selection
        ////////////////////////////////////////////////////////////
        //Pileup
        hNPU = new TH1D("hNPU", "hNPU", 100, 0, 100);
        hNPU->Sumw2();
        hNTrueInt = new TH1D("hNTrueInt", "hNTrueInt", 100, 0, 100);
        hNTrueInt->Sumw2();

        hNPU_after = new TH1D("hNPU_after", "hNPU_after", 100, 0, 100);
        hNPU_after->Sumw2();
        hNTrueInt_after = new TH1D("hNTrueInt_after", "hNTrueInt_after", 100, 0, 100);
        hNTrueInt_after->Sumw2();

        // GenLevel Object histograms
        hGen_Muon_pT  = new TH1D("hGen_Muon_pT",  "hGen_Muon_pT",  4000, 0, 4000);
        hGen_Muon_pT->Sumw2();
        hGen_Muon_phi = new TH1D("hGen_Muon_phi", "hGen_Muon_phi", 72, -M_PI, M_PI);
        hGen_Muon_phi->Sumw2();
        hGen_Muon_eta = new TH1D("hGen_Muon_eta", "hGen_Muon_eta", 50, -2.5, 2.5);
        hGen_Muon_eta->Sumw2();
        hGen_Muon_pT_after = new TH1D("hGen_Muon_pT_after", "hGen_Muon_pT_after", 4000, 0, 4000);
        hGen_Muon_pT_after->Sumw2();
        hGen_Muon_phi_after = new TH1D("hGen_Muon_phi_after", "hGen_Muon_phi_after", 72, -M_PI, M_PI);
        hGen_Muon_phi_after->Sumw2();
        hGen_Muon_eta_after = new TH1D("hGen_Muon_eta_after", "hGen_Muon_eta_after", 50, -2.5, 2.5);
        hGen_Muon_eta_after->Sumw2();

        hGen_Nu_pT  = new TH1D("hGen_Nu_pT",  "hGen_Nu_pT",  4000, 0, 4000);
        hGen_Nu_pT->Sumw2();
        hGen_Nu_phi = new TH1D("hGen_Nu_phi", "hGen_Nu_phi", 72, -M_PI, M_PI);
        hGen_Nu_phi->Sumw2();
        hGen_Nu_eta = new TH1D("hGen_Nu_eta", "hGen_Nu_eta", 50, -2.5, 2.5);
        hGen_Nu_eta->Sumw2();
        hGen_Nu_pT_after = new TH1D("hGen_Nu_pT_after", "hGen_Nu_pT_after", 4000, 0, 4000);
        hGen_Nu_pT_after->Sumw2();
        hGen_Nu_phi_after = new TH1D("hGen_Nu_phi_after", "hGen_Nu_phi_after", 72, -M_PI, M_PI);
        hGen_Nu_phi_after->Sumw2();
        hGen_Nu_eta_after = new TH1D("hGen_Nu_eta_after", "hGen_Nu_eta_after", 50, -2.5, 2.5);
        hGen_Nu_eta_after->Sumw2();

        hGen_MET_phi   = new TH1D("hGen_MET_phi", "hGen_MET_phi", 72, -M_PI, M_PI);
        hGen_MET_phi->Sumw2();
        hGen_MET_pT    = new TH1D("hGen_MET_pT", "hGen_MET_pT", 4000, 0, 4000);
        hGen_MET_pT->Sumw2();
        hGen_MET_phi_after   = new TH1D("hGen_MET_phi_after", "hGen_MET_phi_after", 72, -M_PI, M_PI);
        hGen_MET_phi_after->Sumw2();
        hGen_MET_pT_after    = new TH1D("hGen_MET_pT_after", "hGen_MET_pT_after", 4000, 0, 4000);
        hGen_MET_pT_after->Sumw2();
    }
    // Object histograms
    hMuon_pT  = new TH1D("hMuon_pT", "hMuon_pT", 4000, 0, 4000);
    hMuon_pT->Sumw2();
    hMuon_phi = new TH1D("hMuon_phi", "hMuon_phi", 72, -M_PI, M_PI);
    hMuon_phi->Sumw2();
    hMuon_eta = new TH1D("hMuon_eta", "hMuon_eta", 50, -2.5, 2.5);
    hMuon_eta->Sumw2();
    // Object histograms
    hMuon_pT_after  = new TH1D("hMuon_pT_after", "hMuon_pT_after", 4000, 0, 4000);
    hMuon_pT_after->Sumw2();
    hMuon_phi_after = new TH1D("hMuon_phi_after", "hMuon_phi_after", 72, -M_PI, M_PI);
    hMuon_phi_after->Sumw2();
    hMuon_eta_after = new TH1D("hMuon_eta_after", "hMuon_eta_after", 50, -2.5, 2.5);
    hMuon_eta_after->Sumw2();

    hMET_phi   = new TH1D("hMET_phi", "hMET_phi", 72, -M_PI, M_PI);
    hMET_phi->Sumw2();
    hMET_pT    = new TH1D("hMET_pT", "hMET_pT", 4000, 0, 4000);
    hMET_pT->Sumw2();
    hMET_sumET = new TH1D("hMET_sumET", "hMET_sumET", 4000, 0, 4000);
    hMET_sumET->Sumw2();
    hPFMET_phi   = new TH1D("hPFMET_phi", "hPFMET_phi", 72, -M_PI, M_PI);
    hPFMET_phi->Sumw2();
    hPFMET_pT    = new TH1D("hPFMET_pT", "hPFMET_pT", 4000, 0, 4000);
    hPFMET_pT->Sumw2();
    hPFMET_sumET = new TH1D("hPFMET_sumET", "hPFMET_sumET", 4000, 0, 4000);
    hPFMET_sumET->Sumw2();
    hPFMET_corr_phi   = new TH1D("hPFMET_corr_phi", "hPFMET_corr_phi", 72, -M_PI, M_PI);
    hPFMET_corr_phi->Sumw2();
    hPFMET_corr_pT    = new TH1D("hPFMET_corr_pT", "hPFMET_corr_pT", 4000, 0, 4000);
    hPFMET_corr_pT->Sumw2();
    hPFMET_corr_sumET = new TH1D("hPFMET_corr_sumET", "hPFMET_corr_sumET", 4000, 0, 4000);
    hPFMET_corr_sumET->Sumw2();

    hMET_phi_after   = new TH1D("hMET_phi_after", "hMET_phi_after", 72, -M_PI, M_PI);
    hMET_phi_after->Sumw2();
    hMET_pT_after    = new TH1D("hMET_pT_after", "hMET_pT_after", 4000, 0, 4000);
    hMET_pT_after->Sumw2();
    hMET_sumET_after = new TH1D("hMET_sumET_after", "hMET_sumET_after", 4000, 0, 4000);
    hMET_sumET_after->Sumw2();
    hPFMET_phi_after   = new TH1D("hPFMET_phi_after", "hPFMET_phi_after", 72, -M_PI, M_PI);
    hPFMET_phi_after->Sumw2();
    hPFMET_pT_after    = new TH1D("hPFMET_pT_after", "hPFMET_pT_after", 4000, 0, 4000);
    hPFMET_pT_after->Sumw2();
    hPFMET_sumET_after = new TH1D("hPFMET_sumET_after", "hPFMET_sumET_after", 4000, 0, 4000);
    hPFMET_sumET_after->Sumw2();
    hPFMET_corr_phi_after   = new TH1D("hPFMET_corr_phi_after", "hPFMET_corr_phi_after", 72, -M_PI, M_PI);
    hPFMET_corr_phi_after->Sumw2();
    hPFMET_corr_pT_after    = new TH1D("hPFMET_corr_pT_after", "hPFMET_corr_pT_after", 4000, 0, 4000);
    hPFMET_corr_pT_after->Sumw2();
    hPFMET_corr_sumET_after = new TH1D("hPFMET_corr_sumET_after", "hPFMET_corr_sumET_after", 4000, 0, 4000);
    hPFMET_corr_sumET_after->Sumw2();

    hW_MT            = new TH1D("hW_MT", "hW_MT", 4000, 0, 4000);
    hW_MT->Sumw2();
    hW_MT_after            = new TH1D("hW_MT_after", "hW_MT_after", 4000, 0, 4000);
    hW_MT_after->Sumw2();

    hW_MT_PFMET        = new TH1D("hW_MT_PFMET", "hW_MT_PFMET", 4000, 0, 4000);
    hW_MT_PFMET->Sumw2();
    hW_MT_PFMET_after        = new TH1D("hW_MT_PFMET_after", "hW_MT_PFMET_after", 4000, 0, 4000);
    hW_MT_PFMET_after->Sumw2();

    hW_MT_PFMET_corr        = new TH1D("hW_MT_PFMET_corr", "hW_MT_PFMET_corr", 4000, 0, 4000);
    hW_MT_PFMET_corr->Sumw2();
    hW_MT_PFMET_corr_after        = new TH1D("hW_MT_PFMET_corr_after", "hW_MT_PFMET_corr_after", 4000, 0, 4000);
    hW_MT_PFMET_corr_after->Sumw2();

    // Balance between muon and MET
    hPt_Mu_over_MET = new TH1D("hPt_Mu_over_MET", "hPt_Mu_over_MET", 100, 0, 5);
    hPt_Mu_over_MET->Sumw2();
    hPt_Mu_over_MET_after = new TH1D("hPt_Mu_over_MET_after", "hPt_Mu_over_MET_after", 100, 0, 5);
    hPt_Mu_over_MET_after->Sumw2();

    // Delta phi between muon and MET
    hDeltaPhi_Mu_MET = new TH1D("hDeltaPhi_Mu_MET", "hDeltaPhi_Mu_MET", 72, 0., M_PI);
    hDeltaPhi_Mu_MET->Sumw2();
    hDeltaPhi_Mu_PFMET = new TH1D("hDeltaPhi_Mu_PFMET", "hDeltaPhi_Mu_PFMET", 72, 0., M_PI);
    hDeltaPhi_Mu_PFMET->Sumw2();
    hDeltaPhi_Mu_PFMET_corr = new TH1D("hDeltaPhi_Mu_PFMET_corr", "hDeltaPhi_Mu_PFMET_corr", 72, 0., M_PI);
    hDeltaPhi_Mu_PFMET_corr->Sumw2();
    hDeltaPhi_Mu_MET_after = new TH1D("hDeltaPhi_Mu_MET_after", "hDeltaPhi_Mu_MET_after", 72, 0., M_PI);
    hDeltaPhi_Mu_MET_after->Sumw2();
    hDeltaPhi_Mu_PFMET_after = new TH1D("hDeltaPhi_Mu_PFMET_after", "hDeltaPhi_Mu_PFMET_after", 72, 0., M_PI);
    hDeltaPhi_Mu_PFMET_after->Sumw2();
    hDeltaPhi_Mu_PFMET_corr_after = new TH1D("hDeltaPhi_Mu_PFMET_corr_after", "hDeltaPhi_Mu_PFMET_corr_after", 72, 0., M_PI);
    hDeltaPhi_Mu_PFMET_corr_after->Sumw2();

}

// Write histograms to file
void NanoAnalyzer::WriteHistograms(TFile* f_output) {
    f_output->cd();
    if (isMC) { 
        ////////////////////////////////////////////////////////////
        // GenLevel event weights, before and after each correction
        ////////////////////////////////////////////////////////////
        hGenEvtWeight->Write();

        hNPU->Write();
        hNTrueInt->Write();
        hNPU_after->Write();
        hNTrueInt_after->Write();

        // GenLevel Object histograms
        hGen_Muon_pT->Write();
        hGen_Muon_phi->Write();
        hGen_Muon_eta->Write();
        hGen_Muon_pT_after->Write();
        hGen_Muon_phi_after->Write();
        hGen_Muon_eta_after->Write();

        hGen_Nu_pT->Write();
        hGen_Nu_phi->Write();
        hGen_Nu_eta->Write();
        hGen_Nu_pT_after->Write();
        hGen_Nu_phi_after->Write();
        hGen_Nu_eta_after->Write();

        hGen_MET_phi->Write();
        hGen_MET_pT->Write();
        hGen_MET_phi_after->Write();
        hGen_MET_pT_after->Write();
    }

    hNPV->Write();
    hNPV_after->Write();
    // Object histograms
    hMuon_pT->Write();
    hMuon_phi->Write();
    hMuon_eta->Write();
    hMuon_pT_after->Write();
    hMuon_phi_after->Write();
    hMuon_eta_after->Write();
    hMET_phi->Write();
    hMET_pT->Write();
    hMET_sumET->Write();
    hPFMET_phi->Write();
    hPFMET_pT->Write();
    hPFMET_sumET->Write();
    hPFMET_corr_phi->Write();
    hPFMET_corr_pT->Write();
    hPFMET_corr_sumET->Write();
    hMET_phi_after->Write();
    hMET_pT_after->Write();
    hMET_sumET_after->Write();
    hPFMET_phi_after->Write();
    hPFMET_pT_after->Write();
    hPFMET_sumET_after->Write();
    hPFMET_corr_phi_after->Write();
    hPFMET_corr_pT_after->Write();
    hPFMET_corr_sumET_after->Write();

    hW_MT->Write();
    hW_MT_PFMET->Write();
    hW_MT_PFMET_corr->Write();
    hW_MT_after->Write();
    hW_MT_PFMET_after->Write();
    hW_MT_PFMET_corr_after->Write();

    hPt_Mu_over_MET->Write();
    hPt_Mu_over_MET_after->Write();
    hDeltaPhi_Mu_MET->Write();
    hDeltaPhi_Mu_PFMET->Write();
    hDeltaPhi_Mu_PFMET_corr->Write();
    hDeltaPhi_Mu_MET_after->Write();
    hDeltaPhi_Mu_PFMET_after->Write();
    hDeltaPhi_Mu_PFMET_corr_after->Write();
}

NanoAnalyzer::~NanoAnalyzer() {
    Clear();
}

void NanoAnalyzer::Clear() {
    // delete classes
    delete cData;
    if(isMC){
        delete cPU;
        delete cEfficiencySF;
    }
    delete cRochesterCorrection;
}
