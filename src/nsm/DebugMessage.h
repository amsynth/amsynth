#ifndef __DebugMessage__
#define __DebugMessage__

#include <list>
#include <string>


class DebugMessage {
public:
    DebugMessage (void) ;
    void SendMessage (std::string message) ;
    std::string AddressPattern ;
    std::string TypeTag ;
    std::list<std::string> Arguments ;
} ;

#endif // __DebugMessage__
