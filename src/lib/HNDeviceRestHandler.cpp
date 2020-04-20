#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

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
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    std::ostream& ostr = response.send();

    // Create a json root object
    pjs::Object jsRoot;

    jsRoot.set( "hnodeID", _parent->getHNodeIDStr() );
    jsRoot.set( "crc32ID", _parent->getHNodeIDCRC32Str() );

    jsRoot.set( "name", _parent->getName() );

    try
    {
        // Write out the generated json
        pjs::Stringifier::stringify( jsRoot, ostr, 1 );
    }
    catch( ... )
    {
        ostr << "{\"error\":\"Internal Error\"}";
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
        return NULL;
    }

    // Found, call the create function
    return (it->second)( _parent );
}

