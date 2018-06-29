#ifndef __NsmClient__
#define __NsmClient__

#include <string>
#include <mutex>
#include <condition_variable>
#include "sigs/sigs.h"
#include "DebugMessage.h"

typedef void * nsm_client_t ;

class NsmClient {

public:
  static int openCallback (const char* name, const char* displayName, const char* clientId, char** outMsg, void* userData) ;
  static int saveCallback (char** outMsg, void* userData) ;
  static void activeCallback (int isActive, void* userData) ;
  static DebugMessage debugMessage ;

  NsmClient (std::string programName) ;
  ~NsmClient (void) ;
  int open (const char* name, const char* displayName, const char* clientId) ;
  int save (void) ;
  void active (bool isActive) ;
  void Init (void) ;
  void Debug (std::string message) ;
  void setOpenResult (int result) ;
  void setSaveResult (int result) ;
  void setActiveResult (int result) ;

  std::condition_variable_any openFinished ;
  std::condition_variable_any saveFinished ;
  std::condition_variable_any activeFinished ;

//signals:
  sigs::Signal<void(std::string, std::string, const std::string&)> Open ;
  sigs::Signal<void(void)> Save ;
  sigs::Signal <void(bool&)> Active ;
  sigs::Signal<void(const std::string&)> DebugString ;

private:
  std::string programName ;
  int openResult ;
  int saveResult ;
  int activeResult ;
  nsm_client_t *nsm ;
} ;


#endif //__NsmClient__
