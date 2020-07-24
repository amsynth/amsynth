#ifndef NsmHandler_H
#define NsmHandler_H

#include <string>
#include <fstream>

class NsmClient ;

class NsmHandler {
public:
    static void NsmOpenCallback (void *This, std::string Name, std::string DisplayName, std::string ClientId) ;
    static void NsmSaveCallback (void *This) ;
    static void NsmActiveCallback (void *This, bool isActive) ;

    NsmHandler( NsmClient *nsmClient) ;

    ~NsmHandler( ) ;


private:
    void SaveAll (void) ;
    void SetFilePaths (std::string Path);

    void setNsmClient (NsmClient *client) ;
    void NsmOpen (std::string Name, std::string DisplayName, std::string ClientId) ;
    void NsmSave (void) ;
    void NsmActive (bool isActive) ;
    void Debug (std::string message) ;

    enum class LoadStatus {
        kOk,
        kNoSuchFile,
        kError
    } ;
    LoadStatus loadStatus ;

    NsmClient *nsmClient ;

    std::string RootPath ;
    std::ofstream RootFile ;
    std::string BankPath ;
    std::ofstream BankFile ;
    std::string PresetPath ;
    std::ofstream PresetFile ;

    bool nsmActive ; /**< Set true if running in a Non session */
} ;

#endif //NsmHandler_H
