#if 0
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#endif

#include <iostream>

#include "Poco/String.h"

#include "HNAvahi.h"
//#include "HNRootHandler.h"
#include "HNodeConfig.h"
#include "HNodeDevice.h"

HNodeDevice::HNodeDevice( std::string deviceType, std::string instance )
{
    devType     = deviceType;
    devInstance = instance;
    version     = "2.0.0";

    port        = 8080;

    ownerState  = "available";
}

HNodeDevice::~HNodeDevice()
{

}

std::string 
HNodeDevice::getInstance()
{
    return devInstance;
}

std::string 
HNodeDevice::getDeviceType()
{
    return devType;
}

std::string 
HNodeDevice::getVersionStr()
{
    return version;
}

std::string 
HNodeDevice::getHNodeIDStr()
{
    std::string rstStr;
    hnodeID.getStr( rstStr );
    return rstStr;
}

std::string 
HNodeDevice::getHNodeIDCRC32Str()
{
    return hnodeID.getCRC32AsHexStr();
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

std::string 
HNodeDevice::getOwnerState()
{
    return ownerState;
}

std::string 
HNodeDevice::getOwnerHNodeIDStr()
{
    std::string rstStr;
    ownerHNodeID.getStr( rstStr );
    return rstStr;
}

std::string
HNodeDevice::createAvahiName()
{
    char tmpBuf[256];

    sprintf( tmpBuf, "%s-%s-%s", devType.c_str(), devInstance.c_str(), hnodeID.getCRC32AsHexStr().c_str() );

    return tmpBuf;
}

#define HND_CFGFILE_ROOT_DEFAULT  "/etc/hnode2/"

bool 
HNodeDevice::configExists()
{
    HNodeConfigFile cfgFile;

    cfgFile.setRootPath( HND_CFGFILE_ROOT_DEFAULT );

    return cfgFile.configExists( devType, devInstance );
}

HND_RESULT_T
HNodeDevice::loadConfig()
{
    std::string     rstStr;
    HNodeConfigFile cfgFile;
    HNodeConfig     cfg;

    std::cout << "Loading config..." << std::endl;

    cfgFile.setRootPath( HND_CFGFILE_ROOT_DEFAULT );

    if( cfgFile.loadConfig( devType, devInstance, cfg ) != HNC_RESULT_SUCCESS )
    {
        std::cout << "ERROR: Could not load saved configuration." << std::endl;
        return HND_RESULT_FAILURE;
    }

    HNCSection *secPtr;
    cfg.updateSection( "device", &secPtr );

    // Read out the HNodeID
    if( secPtr->getValueByName( "hnodeID", rstStr ) != HNC_RESULT_SUCCESS )
    {
        return HND_RESULT_FAILURE;
    }

    hnodeID.setFromStr( rstStr );

    // Read the name field
    if( secPtr->getValueByName( "name", rstStr ) != HNC_RESULT_SUCCESS )
    {
        return HND_RESULT_FAILURE;
    }

    setName( rstStr );
  
    return HND_RESULT_SUCCESS;
}

HND_RESULT_T
HNodeDevice::saveConfig()
{
    std::string rstStr;
    HNodeConfigFile cfgFile;
    HNodeConfig     cfg;

    HNCSection *secPtr;
    cfg.updateSection( "device", &secPtr );
 
    hnodeID.getStr( rstStr );
    secPtr->updateValue( "hnodeID", rstStr );

    secPtr->updateValue( "name", getName() );

    //cfg.updateSection( "owner", &secPtr );
    //secPtr->updateValue( "test2", "value4" );
             
    cfgFile.setRootPath( HND_CFGFILE_ROOT_DEFAULT );

    std::cout << "Saving config..." << std::endl;
    if( cfgFile.saveConfig( devType, devInstance, cfg ) != HNC_RESULT_SUCCESS )
    {
        std::cout << "ERROR: Could not save initial configuration." << std::endl;
        return HND_RESULT_FAILURE;
    }

    return HND_RESULT_SUCCESS;
}

void
HNodeDevice::initToDefaults()
{
    // Create a new uuid
    hnodeID.create();

    // Set a default name
    setName( "Name" );
}

void
HNodeDevice::start()
{
    std::string rstStr;

    std::cout << "Looking for config file" << std::endl;
    
    if( configExists() == false )
    {
        initToDefaults();
        saveConfig();
    }

    loadConfig();

    std::cout << "Done loading configuration" << std::endl;

    std::cout << "Starting HNAvahi..." << std::endl;

    avObj.setID( HNODE_DEVICE_AVAHI_TYPE, createAvahiName() );
    avObj.setPort( port );

    hnodeID.getStr( rstStr );    
    avObj.setSrvPair( "hnodeID", rstStr );
    avObj.setSrvPair( "crc32ID", hnodeID.getCRC32AsHexStr() );
    avObj.setSrvPair( "name", getName() );

    avObj.start();

    std::cout << "Started HNAvahi..." << std::endl;

    rest.start( this );
}
