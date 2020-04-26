#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

#include "HNodeID.h"
#include "HNAvahi.h"
#include "HNHttpServer.h"

#define HNODE_DEVICE_AVAHI_TYPE  "_hnode2-rest-http._tcp"

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
        std::string m_dispatchID;
        HNDEPDispatchInf *m_dispatchInf;
        
        std::string m_OpenAPI;

    public:
        HNDEndpoint();
       ~HNDEndpoint();

        void setDispatch( std::string dispatchID, HNDEPDispatchInf *dispatchInf );

        void setOpenAPIJson( std::string OPAJson );

        std::string getDispatchID();

        HNDEPDispatchInf *getDispatchPtr();

        std::string getOpenAPIJson();
};

class HNodeDevice : public HNRestDispatchInterface, public HNDEPDispatchInf
{
    private:
        std::string  devType;
        std::string  devInstance;
        std::string  version;

        uint16_t     port;

        HNodeID      hnodeID;

        std::string  name;

        std::string  ownerState;
        HNodeID      ownerHNodeID;

        std::map< std::string, HNDEndpoint > endpointMap;

        HNAvahi      avObj;

        HNHttpServer rest;

        std::string createAvahiName();

        bool configExists();
        HND_RESULT_T loadConfig();
        HND_RESULT_T saveConfig();

    public:
        HNodeDevice( std::string deviceType, std::string devInstance );
       ~HNodeDevice();

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

        void initToDefaults();

        HND_RESULT_T addEndpoint( HNDEndpoint newEP );

        void start();

        virtual void dispatchEP( HNodeDevice *parent, HNOperationData *opData );

        virtual void restDispatch( HNOperationData *opData );
};

#endif // __HNODE_DEVICE_H__
