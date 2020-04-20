#ifndef _HN_DEVICE_REST_HANDLER_H_
#define _HN_DEVICE_REST_HANDLER_H_

#include <string>
#include <map>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "HNodeDevice.h"

namespace pn = Poco::Net;

// Handle the pre-defined "/hnode2/device/" requests
class HNDeviceRestDevice: public pn::HTTPRequestHandler
{
    public:
        HNDeviceRestDevice( HNodeDevice *parent );

        void handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response );

        static pn::HTTPRequestHandler *create( HNodeDevice *parent );

    private:
        HNodeDevice *_parent;
};

// Handle a "/" level request
class HNDeviceRestRoot: public pn::HTTPRequestHandler
{
    public:
        HNDeviceRestRoot( HNodeDevice *parent );

        void handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response );

        static pn::HTTPRequestHandler *create( HNodeDevice *parent );

    private:
        HNodeDevice *_parent;
};

// A function pointer for the handler creation routine.
// This can then be stored in the factory lookup table.
typedef pn::HTTPRequestHandler* (*HN_DEVHNDLR_CREATE)( HNodeDevice *parent );

// The factory to create http server request handlers.
class HNDeviceRestHandlerFactory: public pn::HTTPRequestHandlerFactory
{
    public:
        HNDeviceRestHandlerFactory( HNodeDevice *parent );

        void registerURI( std::string uri, HN_DEVHNDLR_CREATE createFunc );

        pn::HTTPRequestHandler* createRequestHandler( const pn::HTTPServerRequest& request );

    private:
        HNodeDevice *_parent;

        std::map< std::string, HN_DEVHNDLR_CREATE > uriMap;
};

#endif // _HN_DEVICE_REST_HANDLER_H_
