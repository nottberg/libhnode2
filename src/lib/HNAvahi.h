#ifndef __HN_AVAHI_H__
#define __HN_AVAHI_H__

#include <string>
#include <map>

typedef enum HNAvahiErrorEnum
{
   HNAVAHI_ERR_NONE         = 0,
   HNAVAHI_ERR_GRPNEW       = 1,
   HNAVAHI_ERR_SRVADD       = 2,
   HNAVAHI_ERR_GRPCOMMIT    = 3,
   HNAVAHI_ERR_GRPFAILURE   = 4,
   HNAVAHI_ERR_CLTFAIL      = 5,
   HNAVAHI_ERR_POLLCREATE   = 6,
   HNAVAHI_ERR_CLTCREATE    = 7,
   HNAVAHI_ERR_RUNCREATE    = 8,
   HNAVAHI_ERR_COLLISION    = 9
}HNAVAHI_ERROR_T;

typedef enum HNAvahiStateEnum
{
    HNAVAHI_STATE_IDLE        = 0,
    HNAVAHI_STATE_STARTUP     = 1,
    HNAVAHI_STATE_ESTABLISHED = 2,
    HNAVAHI_STATE_SHUTDOWN    = 3
}HNAVAHI_STATE_T;

class HNAvahi
{
    private:
        void *client; // AvahiClient
        void *group;  // AvahiEntryGroup
        void *simplePoll; // AvahiSimplePoll 
        void *thelp; // HNAvahiRunner

        HNAVAHI_STATE_T state;
        HNAVAHI_ERROR_T failReason;
        std::string     failMsg;

        char     *serviceType;
        char     *serviceName;
        uint16_t  servicePort;
        std::map< std::string, std::string > srvPairs;

        static void callbackEntryGroup( void *grptr, int state, void *userdata ); 
        static void callbackClient( void *clptr, int state, void *userdata ); 

        void createService(); 
        void resetGroup();

        void setState( HNAVAHI_STATE_T value );
        void setFailure( HNAVAHI_ERROR_T reason );

        void cleanup();

        void setGroup( void *value );
        bool hasGroup();

        void setClient( void *value );
        bool hasClient();

    public:
        HNAvahi( std::string srvType, std::string srvName, uint16_t port );
       ~HNAvahi();

        std::string getSrvType();
        std::string getSrvName();

        void setSrvPair( std::string key, std::string value );
        void setSrvTag( std::string key );
        void clearSrvPairs();

        void start();
        void shutdown();

        void runEventLoop();
        void killEventLoop();
};

#endif // __HN_AVAHI_H__
