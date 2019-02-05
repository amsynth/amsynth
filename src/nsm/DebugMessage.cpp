#include <iostream>
#include <sstream>
#include <oscpack/osc/OscOutboundPacketStream.h>
#include <oscpack/ip/UdpSocket.h>

#include "DebugMessage.h"

#define ADDRESS "127.0.0.1"
#define PORT 7001

#define OUTPUT_BUFFER_SIZE 1024

DebugMessage::DebugMessage (void) {
}

void DebugMessage::SendMessage (std::string message) {
    std::list<std::string>::iterator a ;
    std::string::iterator t ;
    std::stringstream Stream  ;

    AddressPattern = "/Message" ;
    TypeTag = "s" ;
    Arguments.clear () ;
    Arguments.push_back (message) ;

    //cout << "Sending Osc Debug Message" << endl ;

    UdpTransmitSocket transmitSocket( IpEndpointName( ADDRESS, PORT ) );
    char buffer[OUTPUT_BUFFER_SIZE];

    osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );

    p << osc::BeginBundleImmediate << osc::BeginMessage( AddressPattern.c_str() ) ;

    for (a=Arguments.begin(), t=TypeTag.begin(); a!=Arguments.end()&&t!=TypeTag.end();) {
        std::string Arg ;

        switch ((*t)) {
            case ',':
            case 'N':
            case 'I': {
                t++ ;
                break ;
            }
            case 'T': {
                std::cout << "True arg: " << std::endl ;
                p << true ;
                t++ ;
                break ;
            }
            case 'F': {
                std::cout << "False arg: " << std::endl ;
                p << false ;
                t++ ;
                break ;
            }
            case 'i': {
                int i ;
                Arg = *a ;

                Stream.clear() ;
                Stream.str("") ;
                Stream << Arg ;
                Stream >> i ;
                p << i ;
                t++ ;
                a++ ;
                break ;
            }
            case 'f': {
                Arg = *a ;

                float f ;
                Stream.clear() ;
                Stream.str("") ;
                Stream << Arg ;
                Stream >> f ;
                p << f ;
                t++ ;
                a++ ;
                break ;
            }
            case 'd': {
                Arg = *a ;

                double d ;
                Stream.clear() ;
                Stream.str("") ;
                Stream << Arg ;
                Stream >> d ;
                p << d ;
                t++ ;
                a++ ;
                break ;
            }
            case 's': {
                std::string s = (*a) ;
                p << s.c_str() ;
                t++ ;
                a++ ;
                break ;
            }
            default: {
                t++ ;
            }
        }
    }

    p << osc::EndMessage << osc::EndBundle ;

    transmitSocket.Send( p.Data(), p.Size() );
}
