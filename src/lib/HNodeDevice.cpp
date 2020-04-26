#include <iostream>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "Poco/String.h"

#include "HNAvahi.h"
#include "HNodeConfig.h"
#include "HNodeDevice.h"

namespace pjs = Poco::JSON;
namespace pdy = Poco::Dynamic;

const std::string g_HNode2DeviceRest = R"(
{
  "openapi": "3.0.0",
  "info": {
    "description": "",
    "version": "1.0.0",
    "title": ""
  },
  "paths": {
    "/hnode2/device/info": {
      "get": {
        "summary": "Get basic information about the device.",
        "operationId": "getDeviceInfo",
        "responses": {
          "200": {
            "description": "successful operation",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object"
                }
              }
            }
          },
          "400": {
            "description": "Invalid status value"
          }
        }
      }
    },

    "/hnode2/device/owner": {
      "get": {
        "summary": "Get information about the owner of this device.",
        "operationId": "getDeviceOwner",
        "responses": {
          "200": {
            "description": "successful operation",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object"
                }
              }
            }
          },
          "400": {
            "description": "Invalid status value"
          }
        }
      }
    },

    "/hnode2/device/endpoint/{epIndx}": {
      "get": {
        "summary": "Get information for a specific endpoint.",
        "operationId": "getSpecificEndpoint",
        "responses": {
          "200": {
            "description": "successful operation",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object"
                }
              }
            }
          },
          "400": {
            "description": "Invalid status value"
          }
        }
      }
    }

  }
}
)";


HNDEndpoint::HNDEndpoint()
{
    m_dispatchInf = NULL;
}

HNDEndpoint::~HNDEndpoint()
{
}

void 
HNDEndpoint::setDispatch( std::string dispatchID, HNDEPDispatchInf *dispatchInf )
{
    m_dispatchID  = dispatchID;
    m_dispatchInf = dispatchInf;
}

void 
HNDEndpoint::setOpenAPIJson( std::string OPAJson )
{
    m_OpenAPI = OPAJson;
}

std::string 
HNDEndpoint::getDispatchID()
{
    return m_dispatchID;
}

HNDEPDispatchInf *
HNDEndpoint::getDispatchPtr()
{
    return m_dispatchInf;
}

std::string 
HNDEndpoint::getOpenAPIJson()
{
    return m_OpenAPI;
}

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


HND_RESULT_T 
HNodeDevice::addEndpoint( HNDEndpoint newEP )
{
    endpointMap.insert( std::pair< std::string, HNDEndpoint >( newEP.getDispatchID(), newEP ) );

    rest.registerEndpointsFromOpenAPI( newEP.getDispatchID(), this, newEP.getOpenAPIJson() );

    return HND_RESULT_SUCCESS;
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
    avObj.setSrvPair( "devType", devType );

    avObj.start();

    std::cout << "Started HNAvahi..." << std::endl;

    HNDEndpoint hndEP;

    hndEP.setDispatch( "hnode2Dev", this );
    hndEP.setOpenAPIJson( g_HNode2DeviceRest ); 

    addEndpoint( hndEP );

    rest.start();
}

void 
HNodeDevice::dispatchEP( HNodeDevice *parent, HNOperationData *opData )
{
    std::cout << "HNodeDevice::dispatchEP() - entry" << std::endl;
    std::cout << "  dispatchID: " << opData->getDispatchID() << std::endl;
    std::cout << "  opID: " << opData->getOpID() << std::endl;

    std::string opID = opData->getOpID();

    if( "getDeviceInfo" == opID )
    {
        // Create a json root object
        pjs::Object jsRoot;

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        jsRoot.set( "hnodeID", getHNodeIDStr() );
        jsRoot.set( "crc32ID", getHNodeIDCRC32Str() );
 
        jsRoot.set( "name", getName() );

        jsRoot.set( "instance", getInstance() );

        jsRoot.set( "deviceType", getDeviceType() );
        jsRoot.set( "version", getVersionStr() );

        // Render the response
        std::ostream& ostr = opData->responseSend();
        try
        {
            // Write out the generated json
            pjs::Stringifier::stringify( jsRoot, ostr, 1 );
        }
        catch( ... )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            return;
        }
    }
    else if( "getDeviceOwner" == opID )
    {
        // Create a json root object
        pjs::Object jsRoot;

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        jsRoot.set( "state", getOwnerState() );
        jsRoot.set( "hnodeID", getOwnerHNodeIDStr() );

        // Render the response
        std::ostream& ostr = opData->responseSend();
        try
        {
            // Write out the generated json
            pjs::Stringifier::stringify( jsRoot, ostr, 1 );
        }
        catch( ... )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            return;
        }
    }
    else
    {
        // Send back not implemented

    }
}

void 
HNodeDevice::restDispatch( HNOperationData *opData )
{
    std::cout << "HNodeDevice::restDispatch() - entry" << std::endl;
    std::cout << "  dispatchID: " << opData->getDispatchID() << std::endl;

    // Find the dispatch destination
    std::map< std::string, HNDEndpoint >::iterator it = endpointMap.find( opData->getDispatchID() );

    // No endpoint found?
    if( it == endpointMap.end() )
    {
        // Respond not implemented
        return;
    }

    // Pass along to the appropriate endpoint
    it->second.getDispatchPtr()->dispatchEP( this, opData );
}

