#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

#include "HNodeID.h"
#include "HNAvahi.h"
#include "HNHttpServer.h"
#include "HNodeConfig.h"
#include "HNFormatStrs.h"
#include "HNDeviceHealth.h"

#define HNODE_DEVICE_AVAHI_TYPE  "_hnode2-rest-http._tcp"
#define HND_CFGFILE_ROOT_DEFAULT  "/var/cache/hnode2/"

// Forward declaration
class HNodeDevice;

typedef enum HNodeDeviceResultEnum
{
    HND_RESULT_SUCCESS,
    HND_RESULT_FAILURE
}HND_RESULT_T;

class HNDEPDispatchInf
{
    private:

    public:
        virtual void dispatchEP( HNodeDevice *parent, HNOperationData *opData ) = 0;
};


class HNDEndpoint
{
    private:
        std::string typeName;
        std::string version;

        std::string m_dispatchID;
        HNDEPDispatchInf *m_dispatchInf;
        
        std::string m_OpenAPI;

    public:
        HNDEndpoint();
       ~HNDEndpoint();

        void setTypeName( std::string value );
        void setVersion( std::string value );

        void setDispatch( std::string dispatchID, HNDEPDispatchInf *dispatchInf );

        void setOpenAPIJson( std::string OPAJson );

        std::string getTypeName();
        std::string getVersion();

        std::string getDispatchID();

        HNDEPDispatchInf *getDispatchPtr();

        std::string getOpenAPIJson();
};

class HNDEventNotifyInf
{
    private:

    public:
        // Settings from the device configuration file have changed.
        virtual void hndnConfigChange( HNodeDevice *parent ) = 0;
};

class HNodeDevice : public HNRestDispatchInterface, public HNDEPDispatchInf
{
    private:
        std::mutex   m_configMutex;
        bool         m_configChange;

        std::string  m_devType;
        std::string  m_devInstance;
        std::string  m_version;

        uint16_t     m_port;

        HNodeID      m_hnodeID;

        std::string  m_name;

        bool         m_owned;
        bool         m_available;
        HNodeID      m_ownerHNodeID;

        std::map< std::string, HNDEndpoint > m_endpointMap;

        HNAvahi      m_avObj;

        HNDEventNotifyInf *m_notifySink;

        HNHttpServer m_rest;

        HNFormatStringStore m_stringStore;

        HNDeviceHealth m_health;

        std::string createAvahiName();

        //bool configExists();
        //HND_RESULT_T loadConfig();
        //HND_RESULT_T saveConfig();

    public:
        HNodeDevice();
        HNodeDevice( std::string deviceType, std::string devInstance );
       ~HNodeDevice();

        void clearNotifySink();
        void setNotifySink( HNDEventNotifyInf *sinkPtr );

        void startConfigAccess();
        void completeConfigAccess();

        void setDeviceType( std::string type );
        void setInstance( std::string instance );
        void setPort( uint16_t port );

        void setName( std::string value );

        void clearOwner();
        void setOwner( std::string hnodeID );

        std::string getInstance();
        std::string getDeviceType();
        std::string getVersionStr();

        std::string getHNodeIDStr();
        std::string getHNodeIDCRC32Str();

        std::string getName();

        uint16_t getPort();

        bool isOwned();
        bool isAvailable();

        std::string getOwnerHNodeIDStr();
        std::string getOwnerCRC32IDStr();

        bool hasHealthMonitoring();
        bool hasEventing();
        bool hasLogging();
        bool hasDataCollection();
        bool hasKeyValue();

        HND_RESULT_T initConfigSections( HNodeConfig &cfg );
        HND_RESULT_T updateConfigSections( HNodeConfig &cfg );
        HND_RESULT_T readConfigSections( HNodeConfig &cfg );

        //void initToDefaults();

        // Register format strings for use by health monitoring and logging
        HND_RESULT_T registerFormatString( std::string formatStr, uint &code );

        // Initialize the Health Monitoring
        HND_RESULT_T enableHealthMonitoring();
        void disableHealthMonitoring();
        HNDeviceHealth& getHealthRef();

        // Add a REST endpoint.
        HND_RESULT_T addEndpoint( HNDEndpoint newEP );

        void start();

        virtual void dispatchEP( HNodeDevice *parent, HNOperationData *opData );

        virtual void restDispatch( HNOperationData *opData );
};

#endif // __HNODE_DEVICE_H__
