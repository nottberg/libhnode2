#include <iostream>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "Poco/String.h"

#include "HNAvahi.h"
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
HNDEndpoint::setTypeName( std::string value )
{
    typeName = value;
}

void 
HNDEndpoint::setVersion( std::string value )
{
    version = value;
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
HNDEndpoint::getTypeName()
{
    return typeName;
}

std::string 
HNDEndpoint::getVersion()
{
    return version;
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

HNodeDevice::HNodeDevice()
{
    m_devType     = "hnode2-default-device";
    m_devInstance = "default";
    m_version     = "2.0.0";

    m_port        = 8080;

    m_ownerState  = "available";

    HNDEndpoint hndEP;

    hndEP.setDispatch( "hnode2Dev", this );
    hndEP.setOpenAPIJson( g_HNode2DeviceRest ); 

    addEndpoint( hndEP );
}

HNodeDevice::HNodeDevice( std::string deviceType, std::string instance )
{
    m_devType     = deviceType;
    m_devInstance = instance;
    m_version     = "2.0.0";

    m_port        = 8080;

    m_ownerState  = "available";

    HNDEndpoint hndEP;

    hndEP.setDispatch( "hnode2Dev", this );
    hndEP.setOpenAPIJson( g_HNode2DeviceRest ); 

    addEndpoint( hndEP );
}

HNodeDevice::~HNodeDevice()
{

}

void
HNodeDevice::setDeviceType( std::string value )
{
    m_devType = value;
}

void 
HNodeDevice::setInstance( std::string value )
{
    m_devInstance = value;
}

std::string 
HNodeDevice::getDeviceType()
{
    return m_devType;
}

std::string 
HNodeDevice::getInstance()
{
    return m_devInstance;
}

std::string 
HNodeDevice::getVersionStr()
{
    return m_version;
}

std::string 
HNodeDevice::getHNodeIDStr()
{
    std::string rstStr;
    m_hnodeID.getStr( rstStr );
    return rstStr;
}

std::string 
HNodeDevice::getHNodeIDCRC32Str()
{
    return m_hnodeID.getCRC32AsHexStr();
}

void 
HNodeDevice::setName( std::string value )
{
    m_name = value;
}

std::string
HNodeDevice::getName()
{
    return m_name;
}

std::string 
HNodeDevice::getOwnerState()
{
    return m_ownerState;
}

std::string 
HNodeDevice::getOwnerHNodeIDStr()
{
    std::string rstStr;
    m_ownerHNodeID.getStr( rstStr );
    return rstStr;
}

std::string
HNodeDevice::createAvahiName()
{
    char tmpBuf[256];

    sprintf( tmpBuf, "%s-%s-%s", m_devType.c_str(), m_devInstance.c_str(), m_hnodeID.getCRC32AsHexStr().c_str() );

    return tmpBuf;
}

HND_RESULT_T 
HNodeDevice::initConfigSections( HNodeConfig &cfg )
{
    std::string rstStr;
    HNCSection *secPtr;

    // Get a pointer to the device section
    cfg.updateSection( "device", &secPtr );

    // Create a new unique hnodeID
    HNodeID hnodeID;
    hnodeID.create();
    hnodeID.getStr( rstStr );
    secPtr->updateValue( "hnodeID", rstStr );

    // Create default name
    secPtr->updateValue( "name", "InitialName" );
}

HND_RESULT_T 
HNodeDevice::updateConfigSections( HNodeConfig &cfg )
{
    std::string rstStr;
    HNCSection *secPtr;

    // Get a pointer to the device section
    cfg.updateSection( "device", &secPtr );

    // Update the hnodeID
    m_hnodeID.getStr( rstStr );
    secPtr->updateValue( "hnodeID", rstStr );

    // Update the name
    secPtr->updateValue( "name", getName() );
}

HND_RESULT_T 
HNodeDevice::readConfigSections( HNodeConfig &cfg )
{
    HNCSection  *secPtr;
    std::string rstStr;

    // Aquire a pointer to the "device" section
    cfg.updateSection( "device", &secPtr );

    // Read out the HNodeID
    if( secPtr->getValueByName( "hnodeID", rstStr ) != HNC_RESULT_SUCCESS )
    {
        return HND_RESULT_FAILURE;
    }

    m_hnodeID.setFromStr( rstStr );

    // Read the name field
    if( secPtr->getValueByName( "name", rstStr ) != HNC_RESULT_SUCCESS )
    {
        return HND_RESULT_FAILURE;
    }

    setName( rstStr );
  
    return HND_RESULT_SUCCESS;
}

#if 0
bool 
HNodeDevice::configExists()
{
    HNodeConfigFile cfgFile;

    cfgFile.setRootPath( HND_CFGFILE_ROOT_DEFAULT );

    return cfgFile.configExists( m_devType, m_devInstance );
}

HND_RESULT_T
HNodeDevice::loadConfig()
{
    std::string     rstStr;
    HNodeConfigFile cfgFile;
    HNodeConfig     cfg;

    std::cout << "Loading config..." << std::endl;

    cfgFile.setRootPath( HND_CFGFILE_ROOT_DEFAULT );

    if( cfgFile.loadConfig( m_devType, m_devInstance, cfg ) != HNC_RESULT_SUCCESS )
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

    m_hnodeID.setFromStr( rstStr );

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
 
    m_hnodeID.getStr( rstStr );
    secPtr->updateValue( "hnodeID", rstStr );

    secPtr->updateValue( "name", getName() );

    //cfg.updateSection( "owner", &secPtr );
    //secPtr->updateValue( "test2", "value4" );
             
    cfgFile.setRootPath( HND_CFGFILE_ROOT_DEFAULT );

    std::cout << "Saving config..." << std::endl;
    if( cfgFile.saveConfig( m_devType, m_devInstance, cfg ) != HNC_RESULT_SUCCESS )
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
    m_hnodeID.create();

    // Set a default name
    setName( "Name" );
}
#endif

HND_RESULT_T 
HNodeDevice::addEndpoint( HNDEndpoint newEP )
{
    m_endpointMap.insert( std::pair< std::string, HNDEndpoint >( newEP.getDispatchID(), newEP ) );

    m_rest.registerEndpointsFromOpenAPI( newEP.getDispatchID(), this, newEP.getOpenAPIJson() );

    return HND_RESULT_SUCCESS;
}

void
HNodeDevice::start()
{
    std::string rstStr;

#if 0
    std::cout << "Looking for config file" << std::endl;
    
    if( configExists() == false )
    {
        initToDefaults();
        saveConfig();
    }

    loadConfig();

    std::cout << "Done loading configuration" << std::endl;
#endif

    std::cout << "Starting HNAvahi..." << std::endl;

    m_avObj.setID( HNODE_DEVICE_AVAHI_TYPE, createAvahiName() );
    m_avObj.setPort( m_port );

    m_hnodeID.getStr( rstStr );    
    m_avObj.setSrvPair( "hnodeID", rstStr );
    m_avObj.setSrvPair( "crc32ID", m_hnodeID.getCRC32AsHexStr() );
    m_avObj.setSrvPair( "name", getName() );
    m_avObj.setSrvPair( "devType", getDeviceType() );

    m_avObj.start();

    std::cout << "Started HNAvahi..." << std::endl;

    m_rest.start();
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
        opData->responseSetStatusAndReason( HNR_HTTP_NOT_IMPLEMENTED );
    }
}

void 
HNodeDevice::restDispatch( HNOperationData *opData )
{
    std::cout << "HNodeDevice::restDispatch() - entry" << std::endl;
    std::cout << "  dispatchID: " << opData->getDispatchID() << std::endl;

    // Find the dispatch destination
    std::map< std::string, HNDEndpoint >::iterator it = m_endpointMap.find( opData->getDispatchID() );

    // No endpoint found?
    if( it == m_endpointMap.end() )
    {
        // Respond not implemented
        return;
    }

    // Pass along to the appropriate endpoint
    it->second.getDispatchPtr()->dispatchEP( this, opData );
}

