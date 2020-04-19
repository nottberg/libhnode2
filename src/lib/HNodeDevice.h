#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

#include "HNodeID.h"
#include "HNAvahi.h"
#include "HNHttpServer.h"

class HNodeDevice
{
    private:
        HNodeID      idObj;

        std::string  instance;
        std::string  name;

        HNAvahi      avObj;

        HNHttpServer rest;

    public:
        HNodeDevice();
       ~HNodeDevice();

        void setName( std::string value );
        std::string getName();

        void start();
};

#endif // __HNODE_DEVICE_H__
