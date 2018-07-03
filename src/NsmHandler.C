#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <csignal>
#include "non/nsm.h"
#include "NsmHandler.h"
#include "main.h"
#include "NsmClient.h"
#include "DebugMessage.h"

using namespace std ;


NsmHandler::NsmHandler( NsmClient *nsmClient) {
  loadStatus = LoadStatus::Ok ;
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
	PresetFile.open (PresetPath) ;
	if (PresetFile.is_open()) {
      Debug ("Saving preset ...") ;
      PresetFile << amsynth_get_preset_number() ;
	  PresetFile.close() ;
	}
  }
}



void NsmHandler::SetFilePaths (string Path) {
  if (!(Path.empty()))  {
    RootPath = BankPath = PresetPath = Path ;
    BankPath.append ("/bank") ;
    PresetPath.append ("/preset_number") ;
  }
}


void NsmHandler::setNsmClient (NsmClient *client) {
  nsmClient = client ;
  nsmClient->Open.connect (this, sigs::Use<string,string,string>::overloadOf (&NsmHandler::NsmOpen)) ;
  nsmClient->Save.connect (this, &NsmHandler::NsmSave) ;
  nsmClient->Active.connect (this, sigs::Use<bool>::overloadOf (&NsmHandler::NsmActive)) ;
  nsmClient->DebugString.connect (this, sigs::Use<string>::overloadOf (&NsmHandler::Debug)) ;
}


void NsmHandler::Debug (string message) {
  nsmClient->Debug (message) ;
}


void NsmHandler::NsmOpen (string Name, string DisplayName, string ClientId) {
  stringstream Stream ;

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

  ifstream bankStream (BankPath.c_str()) ;

  if (bankStream.good()) {
    amsynth_load_bank(BankPath.c_str());

    nsmClient->setOpenResult (ERR_OK) ;
    loadStatus = LoadStatus::Ok ;
  }
  else {
    nsmClient->setOpenResult (ERR_OK) ;
    loadStatus = LoadStatus::NoSuchFile ;
  }

  ifstream presetStream (PresetPath.c_str()) ;

  if (presetStream.good()) {
    int preset_number ;
    presetStream >> preset_number ;
    amsynth_set_preset_number(preset_number);

    nsmClient->setOpenResult (ERR_OK) ;
    loadStatus = LoadStatus::Ok ;
  }
  else {
    nsmClient->setOpenResult (ERR_OK) ;
    loadStatus = LoadStatus::NoSuchFile ;
  }
}


void NsmHandler::NsmSave (void) {
  Debug ("NsmHandler::NsmSave()\n") ;
  if (loadStatus != LoadStatus::Error) {
    SaveAll () ;
    nsmClient->setSaveResult (ERR_OK) ;
  }
  else {
    nsmClient->setSaveResult (ERR_GENERAL) ;
  }
}

void NsmHandler::NsmActive (bool isActive) {
  nsmActive = isActive ;
  Debug ("NsmHandler::NsmActive()\n") ;
  nsmClient->setActiveResult (ERR_OK) ;
}
