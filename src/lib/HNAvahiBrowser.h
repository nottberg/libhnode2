#ifndef __HN_AVAHI_BROWSER_H__
#define __HN_AVAHI_BROWSER_H__

#include <string>
#include <map>

typedef enum HNAvahiBrowserErrorEnum
{
   HNAVAHI_BROWSER_ERR_NONE         = 0,
   HNAVAHI_BROWSER_ERR_GRPNEW       = 1,
   HNAVAHI_BROWSER_ERR_SRVADD       = 2,
   HNAVAHI_BROWSER_ERR_GRPCOMMIT    = 3,
   HNAVAHI_BROWSER_ERR_GRPFAILURE   = 4,
   HNAVAHI_BROWSER_ERR_CLTFAIL      = 5,
   HNAVAHI_BROWSER_ERR_POLLCREATE   = 6,
   HNAVAHI_BROWSER_ERR_CLTCREATE    = 7,
   HNAVAHI_BROWSER_ERR_RUNCREATE    = 8,
   HNAVAHI_BROWSER_ERR_COLLISION    = 9
}HNAVAHI_BROWSER_ERROR_T;

typedef enum HNAvahiBrowserStateEnum
{
    HNAVAHI_BROWSER_STATE_IDLE        = 0,
    HNAVAHI_BROWSER_STATE_STARTUP     = 1,
    HNAVAHI_BROWSER_STATE_ESTABLISHED = 2,
    HNAVAHI_BROWSER_STATE_SHUTDOWN    = 3
}HNAVAHI_BROWSER_STATE_T;

class HNAvahiBrowser
{
    private:
        void *client; // AvahiClient
        void *simplePoll; // AvahiSimplePoll 
        void *serviceBrowser; // AvahiServiceBrowser
        void *thelp; // HNAvahiRunner

        HNAVAHI_BROWSER_STATE_T state;
        HNAVAHI_BROWSER_ERROR_T failReason;

        std::string     failMsg;

        static void callbackResolve( void *r, uint interface, uint protocol, uint event, 
                                     const char *name,  const char *type, const char *domain, 
                                     const char *host_name, const void *address, uint16_t port, 
                                     void *txt, uint flags, void* userdata);

        static void callbackBrowse( void *b, uint interface, uint protocol, uint event, 
                                    const char *name, const char *type, const char *domain, 
                                    uint flags, void* userdata );

        static void callbackClient( void *clptr, int state, void *userdata ); 

        void setState( HNAVAHI_BROWSER_STATE_T value );
        void setFailure( HNAVAHI_BROWSER_ERROR_T reason );

        void cleanup();

        void setClient( void *value );
        bool hasClient();

    public:
        HNAvahiBrowser();
       ~HNAvahiBrowser();

        void start();
        void shutdown();

        void runEventLoop();
        void killEventLoop();
};

#endif // __HN_AVAHI_BROWSER_H__
