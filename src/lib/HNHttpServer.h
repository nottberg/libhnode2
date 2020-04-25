#ifndef _HN_HTTP_SERVER_H_
#define _HN_HTTP_SERVER_H_

#include <string>
//#include "Poco/Util/ServerApplication.h"
//#include "Poco/Net/HTTPRequestHandlerFactory.h"
//#include "Poco/Util/OptionSet.h"

//using Poco::Util::OptionSet;

#include "HNRestHandler.h"
//#include "HNDeviceRestHandler.h"

class HNHttpServer
{
    public:
        HNHttpServer();
       ~HNHttpServer();

        void start( HNRestDispatchInterface *dispatchInf );

    private:
        uint16_t  m_port;
      
        void     *m_srvPtr;
};

#endif
