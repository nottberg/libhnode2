#include "HNodeDevice.h"

HNodeDevice::HNodeDevice()
{

}

HNodeDevice::~HNodeDevice()
{

}

void 
HNodeDevice::setName( std::string value )
{
    name = value;
}

std::string
HNodeDevice::getName()
{
    return name;
}
