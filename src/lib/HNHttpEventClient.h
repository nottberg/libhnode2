#ifndef _HN_HTTP_EVENT_CLIENT_H_
#define _HN_HTTP_EVENT_CLIENT_H_

#include <string>

typedef enum HNHttpEventClientHttpVerbEnum
{
    HNHTTP_VERB_POST,
    HNHTTP_VERB_PUT
}HNHTTP_VERB_T;


class HNHttpEventClient
{
    public:
        HNHttpEventClient();
       ~HNHttpEventClient();

        void submitPostEvent( std::string targetURI, std::string contentType, std::string payload );

    private:

};

#endif // __HN_HTTP_EVENT_CLIENT__
