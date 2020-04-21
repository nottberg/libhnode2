#include "Poco/Net/HTTPServer.h"
//#include "Poco/Net/HTTPRequestHandler.h"
//#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
//#include "Poco/Net/HTTPServerRequest.h"
//#include "Poco/Net/HTTPServerResponse.h"
//#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
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

#include "HNDeviceRestHandler.h"
#include "HNHttpServer.h"

namespace pn = Poco::Net;

HNHttpServer::HNHttpServer()
{
    port   = 8080;
    srvPtr = NULL;
}

HNHttpServer::~HNHttpServer()
{
    if( srvPtr )
        delete ((Poco::Net::HTTPServer*)srvPtr);
}

void
HNHttpServer::start( HNodeDevice *parent )
{
    std::cout << "Starting HttpServer..." << std::endl;

    pn::ServerSocket svs(port);

    HNDeviceRestHandlerFactory *facPtr = new HNDeviceRestHandlerFactory( parent );

    facPtr->registerURI( "/", HNDeviceRestRoot::create );
    facPtr->registerURI( "/hnode2/device/info", HNDeviceRestDevice::create );
    facPtr->registerURI( "/hnode2/device/owner", HNDeviceRestDevice::create );

    srvPtr = (void *) new pn::HTTPServer( facPtr, svs, new pn::HTTPServerParams );
    ((pn::HTTPServer*)srvPtr)->start();
    
     std::cout << "Started HttpServer..." << std::endl; 
}
