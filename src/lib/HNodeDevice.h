#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

#include "HNodeID.h"
#include "HNAvahi.h"
#include "HNHttpServer.h"
#include "HNodeConfig.h"
#include "HNFormatStrs.h"
#include "HNDeviceHealth.h"
#include "HNHostNetwork.h"

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

// Enumeration of the ways services can be specified.
typedef enum 
{
    HNDS_TYPE_NOTSET,     // Default value 
    HNDS_TYPE_DEVICEREST, // An extension of the hnode2 root REST server namespace
    HNDS_TYPE_CUSTOM      // A wholly specified, seperate protocol
}HNDS_TYPE_T;

// Data about a provided/consumed hnode device service interface
class HNDServiceRecord
{
    private:
        HNDS_TYPE_T    m_specType;

        std::string    m_svcType;       
        std::string    m_version;
        std::string    m_extPath;
        std::string    m_customURI;

        bool m_isMapped;

        std::string m_providerCRC32ID;
        std::string m_providerURI;

    public:
        HNDServiceRecord();
       ~HNDServiceRecord();

        void setSpecType( HNDS_TYPE_T stype );

        void setType( std::string value );
        void setVersion( std::string value );
        void setExtPath( std::string value );      
        void setCustomURIFromStr( std::string uri );

        //void setDeviceExtension( bool value );
        void setMapped( bool value );
    
        void setProviderCRC32ID( std::string crc32ID );
        void setProviderURIFromStr( std::string uri );

        HNDS_TYPE_T getSpecType();

        std::string getType();
        std::string getVersion();
        std::string getExtPath();
        std::string getCustomURIAsStr();

        bool isMapped();

        std::string getProviderCRC32ID();
        std::string getProviderURIAsStr();
};

class HNodeDevice : public HNRestDispatchInterface, public HNDEPDispatchInf
{
    private:
        std::mutex   m_configMutex;
        bool         m_configChange;

        std::string  m_devType;
        std::string  m_devInstance;
        std::string  m_version;

        HNHostNetwork m_network;
        std::string   m_address;
        uint16_t      m_port;
        std::string   m_rootPath;

        HNodeID      m_hnodeID;

        std::string  m_name;

        bool         m_owned;
        bool         m_available;
        HNodeID      m_ownerHNodeID;

        std::map< std::string, HNDEndpoint > m_endpointMap;

        HNAvahi      m_avObj;

        HNDEventNotifyInf *m_notifySink;

        HNHttpServer m_rest;

        // Track the services that this hnode provides.
        std::map< std::string, HNDServiceRecord > m_advertisedServices;

        // Track the services that this hnode desires to consume.
        std::map< std::string, HNDServiceRecord > m_desiredServices;

        // Store string references for hnode components.
        HNFormatStringStore m_stringStore;

        // Implement built-in support for hnode health monitoring.
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

        void setRestAddress( std::string address );
        void setRestPort( uint16_t port );
        void setRestRootPath( std::string path );

        void setName( std::string value );

        void clearOwner();
        void setOwner( std::string hnodeID );

        std::string getInstance();
        std::string getDeviceType();
        std::string getVersionStr();

        std::string getHNodeIDStr();
        std::string getHNodeIDCRC32Str();
        uint32_t getHNodeIDCRC32();

        std::string getName();

        std::string getRestAddress();
        uint16_t getRestPort();
        std::string getRestRootPath();

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

        // Clear the provided services map
        void clearProvidedServices();

        // Add provided service advertisements
        void registerProvidedServiceCustom( std::string typeStr, std::string versionStr, std::string uri );
        void registerProvidedServiceExtension( std::string typeStr, std::string versionStr, std::string extPath );

        // Clear a provided service advertisment.
        void clearProvidedService( std::string typeStr );

        // Clear the mapped services map
        void clearDesiredServices();

        // Add a desired service mapping to the mapped services map
        void addDesiredService( std::string typeStr );

        // Clear a single mapped service
        void clearDesiredService( std::string typeStr );

        // Get information about a mapped service.
        bool isServiceMapped( std::string typeStr );

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
