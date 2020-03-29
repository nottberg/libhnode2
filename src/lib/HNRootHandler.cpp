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

#include "HNRootHandler.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;

HNodeRootRequestHandler::HNodeRootRequestHandler(const std::string& format)
: _format(format)
{

}

void 
HNodeRootRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    Application& app = Application::instance();
    app.logger().information("Request from " + request.clientAddress().toString());

    Timestamp now;
    std::string dt(DateTimeFormatter::format(now, _format));

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    std::ostream& ostr = response.send();
    ostr << "<html><head><title>HTTPTimeServer powered by "
        "POCO C++ Libraries</title>";
    ostr << "<meta http-equiv=\"refresh\" content=\"1\"></head>";
    ostr << "<body><p style=\"text-align: center; "
        "font-size: 48px;\">";
    ostr << dt;
    ostr << "</p></body></html>";
}

HNodeRootHandlerFactory::HNodeRootHandlerFactory(const std::string& format)
: _format(format)
{
}

HTTPRequestHandler* 
HNodeRootHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
    if (request.getURI() == "/")
        return new HNodeRootRequestHandler(_format);
    else
        return 0;
}


