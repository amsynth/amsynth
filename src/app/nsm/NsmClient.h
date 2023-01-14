#ifndef __NsmClient__
#define __NsmClient__

#include <string>

typedef void * nsm_client_t ;
typedef void (*HandlerOpenCallback) (void *HandlerThis, std::string Name, std::string DisplayName, std::string ClientId) ;
typedef void (*HandlerSaveCallback) (void *HandlerThis) ;
typedef void (*HandlerActiveCallback) (void *HandlerThis, bool isActive) ;

class NsmClient {

public:
    static int openCallback (const char* name, const char* displayName, const char* clientId, char** outMsg, void* userData) ;
    static int saveCallback (char** outMsg, void* userData) ;
    static void activeCallback (int isActive, void* userData) ;

    NsmClient (std::string programName) ;
    ~NsmClient (void) ;
    int open (const char* name, const char* displayName, const char* clientId) ;
    int save (void) ;
    void active (bool isActive) ;
    void Init (std::string programLabel) ;
    void Debug (std::string message) ;
    void setOpenResult (int result) ;
    void setSaveResult (int result) ;
    void setActiveResult (int result) ;
    void setHandlerOpenCallback (void *handlerThis, HandlerOpenCallback callback) ;
    void setHandlerSaveCallback (void *handlerThis, HandlerSaveCallback callback) ;
    void setHandlerActiveCallback (void *handlerThis, HandlerActiveCallback callback) ;


private:
    std::string programName ;
    int openResult ;
    int saveResult ;
    int activeResult ;
    nsm_client_t *nsm ;
    void *openHandlerThis ;
    void *saveHandlerThis ;
    void *activeHandlerThis ;

    HandlerOpenCallback handlerOpenCallback ;
    HandlerSaveCallback handlerSaveCallback ;
    HandlerActiveCallback handlerActiveCallback ;
} ;


#endif //__NsmClient__
