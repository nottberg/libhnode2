#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
//#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/URI.h"
#if 0
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#endif
#include <iostream>

#include "Poco/String.h"

//#include "HNDeviceRestHandler.h"
#include "HNHttpServer.h"

namespace pn = Poco::Net;

// Define these locally so it is not in the header file.
class HNRestHandler: public pn::HTTPRequestHandler
{
    private:
        HNOperationData *m_opData;

    public:
        HNRestHandler( HNOperationData *operationData );
       ~HNRestHandler();

        void handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response );
};

HNRestHandler::HNRestHandler( HNOperationData *operationData )
:m_opData( operationData )
{

}

HNRestHandler::~HNRestHandler()
{
    std::cout << "HNRestHandler - deconstruct" << std::endl;
    delete m_opData;
}

void
HNRestHandler::handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response )
{
    m_opData->setReqRsp( &request, &response );
    m_opData->restDispatch();
}

class HNRestHandlerFactory: public pn::HTTPRequestHandlerFactory
{
    public:
        HNRestHandlerFactory();

        HNRestPath *addPath( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatchInf );

        pn::HTTPRequestHandler* createRequestHandler( const pn::HTTPServerRequest& request );

    private:
        std::vector< HNRestPath > pathList;
};

HNRestHandlerFactory::HNRestHandlerFactory()
{

}

HNRestPath *
HNRestHandlerFactory::addPath( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatchInf )
{
    HNRestPath newPath;
    HNRestPath *pathPtr;

    // Add new element
    pathList.push_back( newPath );

    // Get pointer
    pathPtr = &pathList.back();
    
    pathPtr->init( dispatchID, operationID, dispatchInf );

    return pathPtr;
}

pn::HTTPRequestHandler* 
HNRestHandlerFactory::createRequestHandler( const pn::HTTPServerRequest& request )
{
    HNOperationData *opData = NULL;
    std::string method;
    std::vector< std::string > pathStrs;

    Poco::URI uri( request.getURI() );    

    //if (request.getURI() == "/")
    //    return new HNodeRestHandler();
    //else

    std::cout << "Server Request method: " << request.getMethod() << std::endl;
    std::cout << "Server Request URI: " << uri.toString() << std::endl;

    uri.getPathSegments( pathStrs );

    for( std::vector< HNRestPath >::iterator it = pathList.begin(); it != pathList.end(); it++ )
    {
        std::cout << "Check handler: " << it->getOpID() << std::endl;
        opData = it->checkForHandler( method, pathStrs );
        if( opData != NULL )
            return new HNRestHandler( opData );
    }

    // Should return default error handler instead?
    return NULL;
}







HNHttpServer::HNHttpServer()
{
    m_port   = 8080;
    m_srvPtr = NULL;

}

HNHttpServer::~HNHttpServer()
{
    if( m_srvPtr )
        delete ((Poco::Net::HTTPServer*)m_srvPtr);
}

void
HNHttpServer::start( HNRestDispatchInterface *dispatchInf )
{
    std::cout << "Starting HttpServer..." << std::endl;

    pn::ServerSocket svs( m_port );

    HNRestPath *path;

    HNRestHandlerFactory *m_facPtr = new HNRestHandlerFactory();

    path = m_facPtr->addPath( "hnode2Dev", "getDeviceInfo",  dispatchInf );
    path->addPathElement( HNRPE_TYPE_PATH, "hnode2" );
    path->addPathElement( HNRPE_TYPE_PATH, "device" );
    path->addPathElement( HNRPE_TYPE_PATH, "info" );

    path = m_facPtr->addPath( "hnode2Dev", "getDeviceOwner", dispatchInf );
    path->addPathElement( HNRPE_TYPE_PATH, "hnode2" );
    path->addPathElement( HNRPE_TYPE_PATH, "device" );
    path->addPathElement( HNRPE_TYPE_PATH, "owner" );

    //facPtr->registerURI( "/", HNDeviceRestRoot::create );
    //facPtr->registerURI( "/hnode2/device/info", HNDeviceRestDevice::create );
    //facPtr->registerURI( "/hnode2/device/owner", HNDeviceRestDevice::create );

    m_srvPtr = (void *) new pn::HTTPServer( m_facPtr, svs, new pn::HTTPServerParams );
    ((pn::HTTPServer*)m_srvPtr)->start();
    
     std::cout << "Started HttpServer..." << std::endl; 
}
