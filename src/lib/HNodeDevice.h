#ifndef __HNODE_DEVICE_H__
#define __HNODE_DEVICE_H__

#include <string>

class HNodeDevice
{
    private:
        std::string name;

    public:
        HNodeDevice();
       ~HNodeDevice();

        void setName( std::string value );
        std::string getName();
};

#endif // __HNODE_DEVICE_H__
