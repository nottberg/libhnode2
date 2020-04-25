#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

#include "HNodeID.h"
#include "HNAvahi.h"
#include "HNHttpServer.h"

#define HNODE_DEVICE_AVAHI_TYPE  "_hnode2-rest-http._tcp"

typedef enum HNodeDeviceResultEnum
{
    HND_RESULT_SUCCESS,
    HND_RESULT_FAILURE
}HND_RESULT_T;

class HNodeDevice : public HNRestDispatchInterface
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

        HNAvahi      avObj;

        HNHttpServer rest;

        std::string createAvahiName();

        bool configExists();
        HND_RESULT_T loadConfig();
        HND_RESULT_T saveConfig();

    public:
        HNodeDevice( std::string deviceType, std::string devInstance );
       ~HNodeDevice();

        std::string getInstance();
        std::string getDeviceType();
        std::string getVersionStr();

        std::string getHNodeIDStr();
        std::string getHNodeIDCRC32Str();

        void setName( std::string value );
        std::string getName();

        void setPort( uint16_t port );
        uint16_t getPort();

        std::string getOwnerState();
        std::string getOwnerHNodeIDStr();

        void initToDefaults();

        void start();

        virtual void restDispatch( HNOperationData *opData );
};

#endif // __HNODE_DEVICE_H__
