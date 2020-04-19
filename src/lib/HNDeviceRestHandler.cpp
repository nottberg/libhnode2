#include "HNDeviceRestHandler.h"

HNDeviceRestDevice::HNDeviceRestDevice( HNodeDevice *parent )
: _parent( parent )
{

}

void 
HNDeviceRestDevice::handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response )
{
    //Application& app = Application::instance();
    //app.logger().information("Request from " + request.clientAddress().toString());

    //Timestamp now;
    //std::string dt(DateTimeFormatter::format(now, _format));

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    std::ostream& ostr = response.send();
    ostr << "{\"test\":\"param\"}";
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
    //Application& app = Application::instance();
    //app.logger().information("Request from " + request.clientAddress().toString());

    //Timestamp now;
    //std::string dt(DateTimeFormatter::format(now, _format));

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    std::ostream& ostr = response.send();
    ostr << "{\"test\":\"param\"}";
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

