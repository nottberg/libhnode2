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

#include "HNRootHandler.h"
#include "HNHttpServer.h"

HNHttpServer::HNHttpServer()
: _helpRequested(false)
{
}

HNHttpServer::~HNHttpServer()
{
}

void 
HNHttpServer::initialize(Application& self)
{
    loadConfiguration();
    ServerApplication::initialize(self);
}

void 
HNHttpServer::uninitialize()
{
    ServerApplication::uninitialize();
}

void 
HNHttpServer::defineOptions(OptionSet& options)
{
    ServerApplication::defineOptions(options);

    options.addOption(
        Option("help", "h", "display argument help information")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<HNHttpServer>(
            this, &HNHttpServer::handleHelp)));
}

void 
HNHttpServer::handleHelp(const std::string& name, 
                const std::string& value)
{
    HelpFormatter helpFormatter(options());
    
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("A web server that serves the current date and time.");
    helpFormatter.format(std::cout);
    stopOptionsProcessing();
    _helpRequested = true;
}

int 
HNHttpServer::main(const std::vector<std::string>& args)
{
    if (!_helpRequested)
    {
        unsigned short port = (unsigned short) config().getInt("HTTPTimeServer.port", 9980);
        std::string format( config().getString("HTTPTimeServer.format", DateTimeFormat::SORTABLE_FORMAT));
    
        ServerSocket svs(port);
        HTTPServer srv( new HNodeRootHandlerFactory(format), svs, new HTTPServerParams );
        srv.start();
        waitForTerminationRequest();
        srv.stop();
    }
    
    return Application::EXIT_OK;   
}
