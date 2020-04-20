#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

#include "HNodeID.h"
#include "HNAvahi.h"
#include "HNHttpServer.h"

typedef enum HNodeDeviceResultEnum
{
    HND_RESULT_SUCCESS,
    HND_RESULT_FAILURE
}HND_RESULT_T;

class HNodeDevice
{
    private:
        std::string  devType;
        std::string  devInstance;

        HNodeID      hnodeID;

        std::string  name;
 
        HNAvahi      avObj;

        uint16_t     port;
        HNHttpServer rest;

        std::string createAvahiName();

        bool configExists();
        HND_RESULT_T loadConfig();
        HND_RESULT_T saveConfig();

    public:
        HNodeDevice( std::string deviceType, std::string devInstance );
       ~HNodeDevice();

        std::string getHNodeIDStr();
        std::string getHNodeIDCRC32Str();

        void setName( std::string value );
        std::string getName();

        void setPort( uint16_t port );
        uint16_t getPort();

        void initToDefaults();

        void start();
};

#endif // __HNODE_DEVICE_H__
