#include <iostream>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>

#include "HNHttpEventClient.h"

namespace pns = Poco::Net;

HNHttpEventClient::HNHttpEventClient()
{

}

HNHttpEventClient::~HNHttpEventClient()
{

}

void
HNHttpEventClient::submitPostEvent( std::string targetURI, std::string contentType, std::string payload )
{
    // FIXME - replace with a threaded implementation
    Poco::URI uri( targetURI );

    std::cout << "submitPostEvent - uri: " << uri.toString() << std::endl;

    pns::HTTPClientSession session( uri.getHost(), uri.getPort() );
    pns::HTTPRequest request( pns::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), pns::HTTPMessage::HTTP_1_1 );
    pns::HTTPResponse response;

    request.setContentType( contentType );

    // Build the json payload to send
    std::ostream& os = session.sendRequest( request );

    // Copy the payload to the http stream.
    os << payload;
 
    std::istream& rs = session.receiveResponse( response );
    std::cout << response.getStatus() << " " << response.getReason() << std::endl;

    if( response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK )
        return;    
}
