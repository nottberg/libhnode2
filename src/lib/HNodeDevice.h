#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

#include "HNodeID.h"
#include "HNAvahi.h"
#include "HNHttpServer.h"

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

class HNodeDevice : public HNRestDispatchInterface, public HNDEPDispatchInf
{
    private:
        std::string  m_devType;
        std::string  m_devInstance;
        std::string  m_version;

        uint16_t     m_port;

        HNodeID      m_hnodeID;

        std::string  m_name;

        std::string  m_ownerState;
        HNodeID      m_ownerHNodeID;

        std::map< std::string, HNDEndpoint > m_endpointMap;

        HNAvahi      m_avObj;

        HNHttpServer m_rest;

        std::string createAvahiName();

        //bool configExists();
        //HND_RESULT_T loadConfig();
        //HND_RESULT_T saveConfig();

    public:
        HNodeDevice();
        HNodeDevice( std::string deviceType, std::string devInstance );
       ~HNodeDevice();

        void setDeviceType( std::string type );
        void setInstance( std::string instance );
        void setPort( uint16_t port );
        void setName( std::string value );

        std::string getInstance();
        std::string getDeviceType();
        std::string getVersionStr();

        std::string getHNodeIDStr();
        std::string getHNodeIDCRC32Str();

        std::string getName();

        uint16_t getPort();

        std::string getOwnerState();
        std::string getOwnerHNodeIDStr();

        HND_RESULT_T initConfigSections( HNodeConfig &cfg );
        HND_RESULT_T updateConfigSections( HNodeConfig &cfg );
        HND_RESULT_T readConfigSections( HNodeConfig &cfg );

        //void initToDefaults();

        HND_RESULT_T addEndpoint( HNDEndpoint newEP );

        void start();

        virtual void dispatchEP( HNodeDevice *parent, HNOperationData *opData );

        virtual void restDispatch( HNOperationData *opData );
};

#endif // __HNODE_DEVICE_H__
