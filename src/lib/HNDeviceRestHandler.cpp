#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

#include "HNDeviceRestHandler.h"

namespace pjs = Poco::JSON;
namespace pdy = Poco::Dynamic;

HNDeviceRestDevice::HNDeviceRestDevice( HNodeDevice *parent )
: _parent( parent )
{

}

void 
HNDeviceRestDevice::handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response )
{
    std::vector< std::string > pSegs;
    Poco::URI uri( request.getURI() );

    if( request.getMethod() == "GET" )
    {
        // Create a json root object
        pjs::Object jsRoot;

        uri.getPathSegments( pSegs );

        if( "info" == pSegs.back() )
        {
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");

            jsRoot.set( "hnodeID", _parent->getHNodeIDStr() );
            jsRoot.set( "crc32ID", _parent->getHNodeIDCRC32Str() );
 
            jsRoot.set( "name", _parent->getName() );

            jsRoot.set( "instance", _parent->getInstance() );

            jsRoot.set( "deviceType", _parent->getDeviceType() );
            jsRoot.set( "version", _parent->getVersionStr() );
        }
        else if( "owner" == pSegs.back() )
        {
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");

            jsRoot.set( "state", _parent->getOwnerState() );
            jsRoot.set( "hnodeID", _parent->getOwnerHNodeIDStr() );
        }
        else
        {
            // Should not occur - URL not implmented.
            response.setStatusAndReason( pn::HTTPServerResponse::HTTP_NOT_IMPLEMENTED );
            return;
        }

        // Render the response
        std::ostream& ostr = response.send();
        try
        {
            // Write out the generated json
            pjs::Stringifier::stringify( jsRoot, ostr, 1 );
        }
        catch( ... )
        {
            response.setStatusAndReason( pn::HTTPServerResponse::HTTP_INTERNAL_SERVER_ERROR );
            return;
        }



    }
    else
    {
         // Indicate method not supported for this URL.
         response.setStatusAndReason( pn::HTTPServerResponse::HTTP_METHOD_NOT_ALLOWED );
         return;
    }
}

pn::HTTPRequestHandler* 
HNDeviceRestDevice::create( HNodeDevice *parent )
{
    return new HNDeviceRestDevice( parent );
}

HNDeviceRestRoot::HNDeviceRestRoot( HNodeDevice *parent )
: _parent( parent )
{

}

void 
HNDeviceRestRoot::handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response )
{
    if( request.getMethod() != "GET" )
    {
         // Indicate method not supported for this URL.
         response.setStatusAndReason( pn::HTTPServerResponse::HTTP_METHOD_NOT_ALLOWED );
         return;
    }

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    std::ostream& ostr = response.send();
    ostr << "<html><body><h2>This is a hnode2 device</h2></body></html>";
}

pn::HTTPRequestHandler* 
HNDeviceRestRoot::create( HNodeDevice *parent )
{
    return new HNDeviceRestRoot( parent );
}

HNDeviceRestHandlerFactory::HNDeviceRestHandlerFactory( HNodeDevice *parent )
: _parent( parent )
{

}

void 
HNDeviceRestHandlerFactory::registerURI( std::string uri, HN_DEVHNDLR_CREATE createFunc )
{
    uriMap.insert( std::pair< std::string, HN_DEVHNDLR_CREATE >( uri, createFunc ) );
}

pn::HTTPRequestHandler* 
HNDeviceRestHandlerFactory::createRequestHandler( const pn::HTTPServerRequest& request )
{
#if 0
    if (request.getURI() == "/")
        return new HNDeviceRestRoot( _parent );
    else
        return 0;
#endif

    // See if the requested uri is supported
    std::map< std::string, HN_DEVHNDLR_CREATE >::iterator it = uriMap.find( request.getURI() );

    if( it == uriMap.end() )
    {
        // Unsupported, return NULL
        pn::HTTPServerResponse& response = request.response();
        response.setStatusAndReason( pn::HTTPServerResponse::HTTP_NOT_IMPLEMENTED );
        return NULL;
    }

    // Found, call the create function
    return (it->second)( _parent );
}

