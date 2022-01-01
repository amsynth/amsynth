#include "NsmHandler.h"

#include "../main.h"
#include "NsmClient.h"

#include <csignal>
#include <iostream>
#include <nsm/nsm.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


NsmHandler::NsmHandler( NsmClient *nsmClient) {
    loadStatus = LoadStatus::kOk ;
    nsmActive = false ;
    setNsmClient (nsmClient) ;
    SetFilePaths ("NotSet") ;
}


NsmHandler::~NsmHandler() {
}


void NsmHandler::SaveAll (void) {
    if (!(BankPath.empty())) {
        Debug ("Saving bank ...") ;
        amsynth_save_bank(BankPath.c_str());
    }

    if (!(PresetPath.empty())) {
        if (PresetFile.is_open())
            PresetFile.close () ;

        PresetFile.open (PresetPath.c_str()) ;

        if (PresetFile.is_open()) {
            Debug ("Saving preset ...") ;
            PresetFile << amsynth_get_preset_number() ;
            PresetFile.close() ;
        }
    }
}



void NsmHandler::SetFilePaths (std::string Path) {
    if (!(Path.empty()))  {
        RootPath = BankPath = PresetPath = Path ;
        BankPath.append ("/bank") ;
        PresetPath.append ("/preset_number") ;
    }
}


void NsmHandler::setNsmClient (NsmClient *client) {
    nsmClient = client ;
    nsmClient->setHandlerOpenCallback (this, NsmHandler::NsmOpenCallback) ;
    nsmClient->setHandlerSaveCallback (this, NsmHandler::NsmSaveCallback) ;
    nsmClient->setHandlerActiveCallback (this, NsmHandler::NsmActiveCallback) ;
}


void NsmHandler::Debug (std::string message) {
    nsmClient->Debug (message) ;
}


void NsmHandler::NsmOpenCallback (void *This, std::string Name, std::string DisplayName, std::string ClientId) {
    NsmHandler *nsmHandler = (NsmHandler*)This ;
    nsmHandler->NsmOpen (Name, DisplayName, ClientId) ;
}

void NsmHandler::NsmOpen (std::string Name, std::string DisplayName, std::string ClientId) {
    std::stringstream Stream ;

    Stream << "NsmHandler::NsmOpen()\n" ;
    Stream << "Name: " << Name << "\n" ;
    Stream << "DisplayName: " << DisplayName << "\n" ;
    Stream << "ClientId: " << ClientId << "\n" ;
    Debug (Stream.str()) ;

    // Set the paths of the bank and preset files.
    SetFilePaths (Name) ;

    // Create a unique directory in which to create the bank and preset files
    // Note mkdir is unix-specific; please add other os versions here for different platforms.
    mkdir (RootPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) ;

    std::ifstream bankStream (BankPath.c_str()) ;

    if (bankStream.good()) {
        amsynth_load_bank(BankPath.c_str());

        nsmClient->setOpenResult (ERR_OK) ;
        loadStatus = LoadStatus::kOk ;
    }
    else {
        nsmClient->setOpenResult (ERR_OK) ;
        loadStatus = LoadStatus::kNoSuchFile ;
    }

    std::ifstream presetStream (PresetPath.c_str()) ;

    if (presetStream.good()) {
        int preset_number ;
        presetStream >> preset_number ;
        amsynth_set_preset_number(preset_number);

        nsmClient->setOpenResult (ERR_OK) ;
        loadStatus = LoadStatus::kOk ;
    }
    else {
        nsmClient->setOpenResult (ERR_OK) ;
        loadStatus = LoadStatus::kNoSuchFile ;
    }
}


void NsmHandler::NsmSaveCallback (void *This) {
    NsmHandler *nsmHandler = (NsmHandler*)This ;
    nsmHandler->NsmSave () ;
}

void NsmHandler::NsmSave (void) {
    Debug ("NsmHandler::NsmSave()\n") ;
    if (loadStatus != LoadStatus::kError) {
        SaveAll () ;
        nsmClient->setSaveResult (ERR_OK) ;
    }
    else {
        nsmClient->setSaveResult (ERR_GENERAL) ;
    }
}

void NsmHandler::NsmActiveCallback (void *This, bool isActive) {
    NsmHandler *nsmHandler = (NsmHandler*)This ;
    nsmHandler->NsmActive (isActive) ;
}

void NsmHandler::NsmActive (bool isActive) {
    nsmActive = isActive ;
    Debug ("NsmHandler::NsmActive()\n") ;
    nsmClient->setActiveResult (ERR_OK) ;
}
