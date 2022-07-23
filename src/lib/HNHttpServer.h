#ifndef _HN_HTTP_SERVER_H_
#define _HN_HTTP_SERVER_H_

#include <string>

#include "HNRestHandler.h"

class HNHttpServer
{
    public:
        HNHttpServer();
       ~HNHttpServer();

        void setPort(uint16_t port);
        uint16_t getPort();
        
        void registerEndpointsFromOpenAPI( std::string dispatchID, HNRestDispatchInterface *dispatchInf, std::string openAPIJson );

        void start();

    private:
        uint16_t m_port;

        void *m_facPtr;

        void *m_srvPtr;
};

#endif
