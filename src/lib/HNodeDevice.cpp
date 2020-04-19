#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>

#include "Poco/String.h"

#include "HNAvahi.h"
#include "HNRootHandler.h"
#include "HNodeDevice.h"

HNodeDevice::HNodeDevice()
: avObj( "_hnode2-http._tcp", "hnode2TestApp", 651 )
{

}

HNodeDevice::~HNodeDevice()
{

}

void 
HNodeDevice::setName( std::string value )
{
    name = value;
}

std::string
HNodeDevice::getName()
{
    return name;
}

void
HNodeDevice::start()
{
    //HNAvahi avObj( "_hnode2-http._tcp", "hnode2TestApp", 651 );

    std::cout << "Starting HNAvahi..." << std::endl;

    avObj.setSrvPair( "hnodeid", "12:34:45:67:89:01:23:45:12:34:45:67:89:01:23:45" );
    avObj.setSrvPair( "paired", "false" );
    avObj.setSrvTag( "tst-tag" );

    avObj.start();


    std::cout << "Started HNAvahi..." << std::endl;
}
