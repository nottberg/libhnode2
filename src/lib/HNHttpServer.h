#ifndef _HN_HTTP_SERVER_H_
#define _HN_HTTP_SERVER_H_

#include <string>
//#include "Poco/Util/ServerApplication.h"
//#include "Poco/Net/HTTPRequestHandlerFactory.h"
//#include "Poco/Util/OptionSet.h"

//using Poco::Util::OptionSet;

// Forward declaration for back pointer
class HNodeDevice;

class HNHttpServer
{
    public:
        HNHttpServer();
       ~HNHttpServer();

        void start( HNodeDevice *parent );

    private:
        uint16_t  port;
        void     *srvPtr;
};

#endif
