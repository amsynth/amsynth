#include "NsmClient.h"

#include "../main.h"

#include <nsm/nsm.h>
#include <sstream>


static std::string to_string(int intValue) {
    std::ostringstream stream;
    stream << intValue;
    return stream.str();
}


NsmClient *NSMClient ;

void NsmClient::Debug (std::string message) {
}

int NsmClient::openCallback (const char *path, const char *displayName, const char *clientId, char **outMsg, void *userData) {
    (void)outMsg ;

    NsmClient *nsmClient = (NsmClient*)userData ;
    nsmClient->Debug ("NsmClient: openCallback()\n") ;
    return nsmClient->open (path, displayName, clientId) ;                                  
}                                                         
                                                          
int NsmClient::saveCallback (char **outMsg, void *userData) {
    (void)outMsg ;

    NsmClient *nsmClient = (NsmClient*)userData ;
    nsmClient->Debug ("NsmClient: saveCallback()\n") ;
    return nsmClient->save () ;                                      
}                                                         
                                                          
void NsmClient::activeCallback (int isActive, void *userData) {
    NsmClient *nsmClient = (NsmClient*)userData ;
    nsmClient->Debug ("NsmClient: activeCallback()\n") ;

    if (isActive == 1) {
        nsmClient->Debug ("Nsm is active\n") ;
        nsmClient->active (true) ;
    }
    else {
      nsmClient->Debug ("Nsm is NOT active\n") ;
      nsmClient->active (false) ;
    }
}                                                         

void NsmClient::setOpenResult (int result) {
    openResult = result ;
}

void NsmClient::setSaveResult (int result) {
    saveResult = result ;
}

void NsmClient::setActiveResult (int result) {
    activeResult = result ;
}

void NsmClient::setHandlerOpenCallback (void *handlerThis, HandlerOpenCallback callback) {
    openHandlerThis = handlerThis ;
    handlerOpenCallback = callback ;
}

void NsmClient::setHandlerSaveCallback (void *handlerThis, HandlerSaveCallback callback) {
    saveHandlerThis = handlerThis ;
    handlerSaveCallback = callback ;
}

void NsmClient::setHandlerActiveCallback (void *handlerThis, HandlerActiveCallback callback) {
    activeHandlerThis = handlerThis ;
    handlerActiveCallback = callback ;
}
                                                          
NsmClient::NsmClient (std::string programName) : nsm(nullptr) {
    NSMClient = this ;
    this->programName = programName ;

    openResult = -1 ;
    saveResult = -1 ;
    activeResult = -1 ;
}

void NsmClient::Init (std::string programLabel) {
    Debug ("NsmClient: Constructor()\n") ;

    const char *nsm_url = getenv("NSM_URL") ;

    if (nsm_url) {
        Debug ("nsm_url: ") ;
        Debug (std::string(nsm_url)) ;
        Debug ("\n") ;

        nsm = nsm_new ();

        nsm_set_open_callback (nsm, openCallback, this) ;
        nsm_set_save_callback (nsm, saveCallback, this) ;
        nsm_set_active_callback (nsm, activeCallback, this) ;

        if (nsm_init_thread (nsm, nsm_url) == 0) {
            nsm_send_announce( nsm, programLabel.c_str(), "", programName.c_str() ) ;
            nsm_thread_start (nsm) ;
        }
        else {
            nsm_free( nsm ) ;
            nsm = nullptr ;
        }

        Debug ("NsmClient: Registered Callbacks\n") ;
    }
}



NsmClient::~NsmClient (void) {
    Debug ("NsmClient: Destructor()\n") ;
    if (nsm != nullptr) {
        nsm_thread_stop (nsm) ;
        nsm_free( nsm ) ;
    }
}

int NsmClient::open (const char* name, const char* displayName, const char* clientId) {
    Debug ("NsmClient: open()\n") ;
    handlerOpenCallback (openHandlerThis, std::string(name), std::string(displayName), std::string(clientId)) ;

    Debug ("Open Result: ") ;
    Debug (to_string (openResult)) ;
    Debug ("\n") ;

    return openResult ;
}

int NsmClient::save (void) {
    Debug ("NsmClient: save()\n") ;
    handlerSaveCallback (saveHandlerThis) ;

    Debug ("Save Result: ") ;
    Debug (to_string (saveResult)) ;
    Debug ("\n") ;

    return saveResult ;
}

void NsmClient::active (bool isActive) {
    Debug ("NsmClient: active()\n") ;
    handlerActiveCallback (activeHandlerThis, isActive) ;

    Debug ("Active Result: ") ;
    Debug (to_string (activeResult)) ;
    Debug ("\n") ;
}

