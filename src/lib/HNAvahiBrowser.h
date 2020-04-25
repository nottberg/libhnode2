#ifndef __HN_AVAHI_BROWSER_H__
#define __HN_AVAHI_BROWSER_H__

#include <string>
#include <map>

#include "HNSigSyncQueue.h"

typedef enum HNAvahiBrowserEventType
{
    HNAB_EVTYPE_NOTSET,
    HNAB_EVTYPE_ADD,
    HNAB_EVTYPE_REMOVE
} HNAB_EVTYPE_T; 

class HNAvahiBrowserEvent
{
    private:
        HNAB_EVTYPE_T evType;
        std::string   name;
        std::string   type;
        std::string   domain;

        std::string   hostname;
        std::string   address;
        uint16_t      port;

        std::map< std::string, std::string > txtPairs;

    public:
        HNAvahiBrowserEvent();
       ~HNAvahiBrowserEvent();

        void clear();

        void setAddEvent( std::string name, std::string type, std::string domain, std::string hostname, std::string address, uint16_t port );
        void setRemoveEvent( std::string name, std::string type, std::string domain );

        void addTxtPair( std::string key, std::string value ); 

        void setName( std::string value );

        HNAB_EVTYPE_T getEventType();
        std::string getName();

        std::string getTxtValue( std::string key );

        void debugPrint();
};

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
        std::string devType;

        HNAVAHI_BROWSER_STATE_T state;

        HNAVAHI_BROWSER_ERROR_T failReason;
        std::string     failMsg;

        // Pass discovery related events back 
        // to caller.
        HNSigSyncQueue eventQueue;

        void *client; // AvahiClient
        void *simplePoll; // AvahiSimplePoll 
        void *serviceBrowser; // AvahiServiceBrowser
        void *thelp; // HNAvahiRunner

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
        HNAvahiBrowser( std::string avahiType );
       ~HNAvahiBrowser();

        HNSigSyncQueue& getEventQueue();

        void start();
        void shutdown();

        void runEventLoop();
        void killEventLoop();
};

#endif // __HN_AVAHI_BROWSER_H__
