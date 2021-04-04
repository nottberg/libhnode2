#ifndef _HN_DEVICE_REST_HANDLER_H_
#define _HN_DEVICE_REST_HANDLER_H_

#include <string>
#include <map>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "HNRestHandler.h"

class HNodeDevice;

namespace pn = Poco::Net;

// Handle the pre-defined "/hnode2/device/" requests
class HNDRestBuiltInHandler: public pn::HTTPRequestHandler
{
    private:
        HNodeDevice     *m_parent;
        HNOperationData *m_opData;

    public:
        HNDRestBuiltInHandler( HNodeDevice *parent, HNOperationData *opData );

        void handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response );
};

class HNDRestBuiltInFactory : public HNRestHandlerFactoryInterface
{
    private:
        HNodeDevice *m_parent;

    public:
        HNDRestBuiltInFactory( HNodeDevice *parent );
       ~HNDRestBuiltInFactory();

        virtual pn::HTTPRequestHandler* createRequestHandler( HNOperationData *opData );
};

#if 0
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
#endif

#endif // _HN_DEVICE_REST_HANDLER_H_
