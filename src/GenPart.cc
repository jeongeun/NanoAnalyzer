#include "GenPart.h"

// This is for debugging purposes
void GenPartHolder::PrintGenPartInfo() {
    std::cout << "GenPart_idx: " << iGenPartIdx << std::endl;
    std::cout << "GenPart_charge: " << iGenPartCharge << std::endl;
    std::cout << "GenPart_pdgId: " << iGenPartPDGID << std::endl;
    std::cout << "GenPart_status: " << iGenPartStatus << std::endl;
    std::cout << "GenPart_statusFlags: " << iGenPartStatusFlags << std::endl;
    std::cout << "GenPart_motherIdx: " << iGenPartMotherIdx << std::endl;
    std::cout << "GenPart_motherPDGID: " << iGenPartMotherPDGID << std::endl;
    std::cout << "GenPart_motherStatus: " << iGenPartMotherStatus << std::endl;
}

// This is for debugging purposes
void GenPartHolder::PrintGenPartStatusFlags() {
    // Define an array of the corresponding flag names
    const char* flagNames[] = {
        "isPrompt",
        "isDecayedLeptonHadron",
        "isTauDecayProduct",
        "isPromptTauDecayProduct",
        "isDirectTauDecayProduct",
        "isDirectPromptTauDecayProduct",
        "isDirectHadronDecayProduct",
        "isHardProcess",
        "fromHardProcess",
        "isHardProcessTauDecayProduct",
        "isDirectHardProcessTauDecayProduct",
        "fromHardProcessBeforeFSR",
        "isFirstCopy",
        "isLastCopy",
        "isLastCopyBeforeFSR"
    };

    // Print the header
    std::cout << "GenPart_statusFlags: " << std::bitset<15>(iGenPartStatusFlags) << std::endl;
    std::cout << "Flags:" << std::endl;

    // Iterate through each bit and print its value and name
    for (int i = 0; i < 15; ++i) {
        if (((iGenPartStatusFlags >> i) & 1))
        std::cout << flagNames[i] << ": " << ((iGenPartStatusFlags >> i) & 1) << std::endl;
    }
}

bool GenPartHolder::IsPrompt() {
    // Check if the least significant bit (bit 0) is set
    return (iGenPartStatusFlags & 0b0000000000000001) != 0;
}

bool GenPartHolder::IsHardProcess() {
    return (iGenPartStatusFlags & (1 << 7)) != 0;
}

bool GenPartHolder::IsFromHardProcess() {
    // Check if bit 8 (fromHardProcess) is set
    return (iGenPartStatusFlags & (1 << 8)) != 0;
}

bool GenPartHolder::IsTauDecayProduct() {
    // Check if bit 3 (isTauDecayProduct) is set
    return (iGenPartStatusFlags & (1 << 2)) != 0;
}

bool GenPartHolder::IsPromptTauDecayProduct() {
    // Check if bit 4 (isPromptTauDecayProduct) is set
    return (iGenPartStatusFlags & (1 << 3)) != 0;
}

bool GenPartHolder::IsDirectTauDecayProduct() {
    // Check if bit 5 (isDirectTauDecayProduct) is set
    return (iGenPartStatusFlags & (1 << 4)) != 0;
}  

bool GenPartHolder::IsDirectPromptTauDecayProduct() {
    // Check if bit 6 (isDirectPromptTauDecayProduct) is set
    return (iGenPartStatusFlags & (1 << 5)) != 0;
}




void GenParts::Init() {
    if (isinit) {
        std::cerr << "[Warning] GenParts::Init() - GenParts is already initialized" << std::endl;
        return;
    }
    vGenPartVec.clear();
    vGenPartVec.reserve(**(cData->nGenPart)); 
    // Set the initialization flag

    for (Int_t idx = 0; idx < **(cData->nGenPart); idx++) {
        TLorentzVector vec;
        vec.SetPtEtaPhiM(cData->GenPart_pt->At(idx), cData->GenPart_eta->At(idx), cData->GenPart_phi->At(idx), cData->GenPart_mass->At(idx));
        GenPartHolder genPart(vec, idx, (int) -1 * (cData->GenPart_pdgId->At(idx) / std::abs(cData->GenPart_pdgId->At(idx))), cData->GenPart_pdgId->At(idx), cData->GenPart_status->At(idx), cData->GenPart_statusFlags->At(idx));
        genPart.SetGenPartMotherIdx(cData->GenPart_genPartIdxMother->At(idx));
        genPart.SetGenPartMotherPDGID(cData->GenPart_pdgId->At(cData->GenPart_genPartIdxMother->At(idx)));
        genPart.SetGenPartMotherStatus(cData->GenPart_status->At(cData->GenPart_genPartIdxMother->At(idx)));
        vGenPartVec.push_back(genPart);
    }


    isinit = true;
}

std::vector<GenPartHolder>& GenParts::GetGenParts() {
    if (!isinit) {
        std::cerr << "[ERROR] GenParts::GetGenParts() - GenParts are not initialized" << std::endl;
        return vGenPartVec;
    }
    return vGenPartVec;
}

std::vector<GenPartHolder> GenParts::GetGenMuons() {
    std::vector<GenPartHolder> vGenMuons;
    for (auto& genparticle : vGenPartVec) {
        if (std::abs(genparticle.GetGenPartPDGID()) == 13) {
            vGenMuons.push_back(genparticle);
        }
    }
    return vGenMuons;
}

std::vector<GenPartHolder> GenParts::GetGenElectrons() {
    std::vector<GenPartHolder> vGenElectrons;
    for (auto& genparticle : vGenPartVec) {
        if (std::abs(genparticle.GetGenPartPDGID()) == 11) {
            vGenElectrons.push_back(genparticle);
        }
    }
    return vGenElectrons;
}

std::vector<GenPartHolder> GenParts::GetGenTaus() {
    std::vector<GenPartHolder> vGenTaus;
    for (auto& genparticle : vGenPartVec) {
        if (std::abs(genparticle.GetGenPartPDGID()) == 15) {
            vGenTaus.push_back(genparticle);
        }
    }
    return vGenTaus;
}

std::vector<GenPartHolder> GenParts::GetGenNeutrinos() {
    std::vector<GenPartHolder> vGenNeutrinos;
    for (auto& genparticle : vGenPartVec) {
        if (std::abs(genparticle.GetGenPartPDGID()) == 12 || std::abs(genparticle.GetGenPartPDGID()) == 14 || std::abs(genparticle.GetGenPartPDGID()) == 16) {
            vGenNeutrinos.push_back(genparticle);
        }
    }
    return vGenNeutrinos;
}

void GenParts::PrintGenPartChain() {
    for (auto& genparticle : vGenPartVec) {
        Int_t pdgId = genparticle.GetGenPartPDGID();
        Int_t idx   = genparticle.GetGenPartIdx();
        Int_t status = genparticle.GetGenPartStatus();
        Int_t statusFlags = genparticle.GetGenPartStatusFlags();
        Int_t motherIdx = genparticle.GetGenPartMotherIdx();
        Int_t motherPDGID = genparticle.GetGenPartMotherPDGID();
        Int_t motherStatus = genparticle.GetGenPartMotherStatus();

        if (std::abs(pdgId) == 11) {
            std::cout << "Idx: " << idx << " Elec status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 13) {
            std::cout << "Idx: " << idx << " Muon status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 15) {
            std::cout << "Idx: " << idx << " Tau status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 12) {
            std::cout << "Idx: " << idx << " Nu(e) status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 14) {
            std::cout << "Idx: " << idx << " Nu(m) status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 16) {
            std::cout << "Idx: " << idx << " Nu(t) status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 22) {
            std::cout << "Idx: " << idx << " Gamma status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 23) {
            std::cout << "Idx: " << idx << " Z boson status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 24) {
            std::cout << "Idx: " << idx << " W boson status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else {
            std::cout << "Idx: " << idx << " PDGID: " << pdgId << " status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
    }
}
