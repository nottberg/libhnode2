#include <iostream>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "Poco/String.h"

#include "HNAvahi.h"
#include "HNodeDevice.h"

namespace pjs = Poco::JSON;
namespace pdy = Poco::Dynamic;

// Forward declaration
extern const std::string g_HNode2DeviceRest;

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
: m_health( &m_stringStore )
{
    m_configChange = false;
    m_notifySink = NULL;

    m_devType     = "hnode2-default-device";
    m_devInstance = "default";
    m_version     = "2.0.0";

    m_port        = 8080;

    m_owned       = false;
    m_available   = true;

    HNDEndpoint hndEP;

    hndEP.setDispatch( "hnode2Dev", this );
    hndEP.setOpenAPIJson( g_HNode2DeviceRest ); 

    addEndpoint( hndEP );
}

HNodeDevice::HNodeDevice( std::string deviceType, std::string instance )
: m_health( &m_stringStore )
{
    m_configChange = false;
    m_notifySink = NULL;

    m_devType     = deviceType;
    m_devInstance = instance;
    m_version     = "2.0.0";

    m_port        = 8080;

    m_owned       = false;
    m_available   = true;

    HNDEndpoint hndEP;

    hndEP.setDispatch( "hnode2Dev", this );
    hndEP.setOpenAPIJson( g_HNode2DeviceRest ); 

    addEndpoint( hndEP );
}

HNodeDevice::~HNodeDevice()
{

}

void
HNodeDevice::clearNotifySink()
{
    m_notifySink = NULL;
}

void
HNodeDevice::setNotifySink( HNDEventNotifyInf *sinkPtr )
{
    m_notifySink = sinkPtr;
}

void
HNodeDevice::startConfigAccess()
{
    m_configMutex.lock();
    m_configChange = false;
}

void
HNodeDevice::completeConfigAccess()
{
    if( (m_notifySink != NULL) && (m_configChange == true) )
    {
        m_notifySink->hndnConfigChange( this );
        m_configChange = false;
    }

    m_configMutex.unlock();
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

void 
HNodeDevice::setName( std::string value )
{
    m_name = value;
    m_configChange = true;
}

void 
HNodeDevice::setPort( uint16_t port )
{
    m_port = port;
}

void 
HNodeDevice::clearOwner()
{
    m_owned = false;
    m_ownerHNodeID.clear();
    m_configChange = true;
}

void
HNodeDevice::setOwner( std::string hnodeID )
{
    m_owned = true;
    m_ownerHNodeID.setFromStr( hnodeID );
    m_configChange = true;
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

std::string
HNodeDevice::getName()
{
    return m_name;
}

uint16_t
HNodeDevice::getPort()
{
    return m_port;
}

bool
HNodeDevice::isOwned()
{
    return m_owned;
}

bool
HNodeDevice::isAvailable()
{
    return m_available;
}

std::string 
HNodeDevice::getOwnerHNodeIDStr()
{
    std::string rstStr;
    m_ownerHNodeID.getStr( rstStr );
    return rstStr;
}

std::string 
HNodeDevice::getOwnerCRC32IDStr()
{
    return m_ownerHNodeID.getCRC32AsHexStr();
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

    std::lock_guard<std::mutex> lock( m_configMutex );

    // Get a pointer to the device section
    cfg.updateSection( "device", &secPtr );

    // Create a new unique hnodeID
    HNodeID hnodeID;
    hnodeID.create();
    hnodeID.getStr( rstStr );
    secPtr->updateValue( "hnodeID", rstStr );

    // Create default name
    secPtr->updateValue( "name", "InitialName" );

    return HND_RESULT_SUCCESS;
}

HND_RESULT_T 
HNodeDevice::updateConfigSections( HNodeConfig &cfg )
{
    std::string rstStr;
    HNCSection *secPtr;

    std::lock_guard<std::mutex> lock( m_configMutex );

    // Get a pointer to the device section
    cfg.updateSection( "device", &secPtr );

    // Update the hnodeID
    m_hnodeID.getStr( rstStr );
    secPtr->updateValue( "hnodeID", rstStr );

    // Update the name
    secPtr->updateValue( "name", getName() );

    // If we are claimed then add owner info
    if( m_owned == true )
    {
        m_ownerHNodeID.getStr( rstStr );
        secPtr->updateValue( "owner_hnodeID", rstStr );
    }

    return HND_RESULT_SUCCESS;
}

HND_RESULT_T 
HNodeDevice::readConfigSections( HNodeConfig &cfg )
{
    HNCSection  *secPtr;
    std::string rstStr;

    std::lock_guard<std::mutex> lock( m_configMutex );

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
  
    // Read the name field
    if( secPtr->getValueByName( "owner_hnodeID", rstStr ) == HNC_RESULT_SUCCESS )
    {
        setOwner( rstStr );
    }
    else
    {
        clearOwner();
    }

    return HND_RESULT_SUCCESS;
}

HND_RESULT_T
HNodeDevice::registerFormatString( std::string formatStr, uint &code )
{
    if( m_stringStore.registerFormatString( formatStr, code ) != HNFS_RESULT_SUCCESS )
      return HND_RESULT_FAILURE;

    return HND_RESULT_SUCCESS;
}

HND_RESULT_T
HNodeDevice::enableHealthMonitoring()
{
    m_health.setEnabled(); 
    return HND_RESULT_SUCCESS;
}

void
HNodeDevice::disableHealthMonitoring()
{
    m_health.clear();
}

HNDeviceHealth& 
HNodeDevice::getHealthRef()
{
    return m_health;
}

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

    if( m_health.isEnabled() == true )
        m_health.init( createAvahiName(), m_hnodeID.getCRC32AsHexStr(), getName() );

    m_rest.setPort( m_port );
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
    else if( "putDeviceConfig" == opID )
    {
        std::istream& bodyStream = opData->requestBody();

        // Prepare for object update
        startConfigAccess();

        // Parse the json body of the request
        try
        {
            // Attempt to parse the json
            pjs::Parser parser;
            pdy::Var varRoot = parser.parse( bodyStream );

            // Get a pointer to the root object
            pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

            if( jsRoot->has( "name" ) )
            {
                setName( jsRoot->getValue<std::string>( "name" ) );
            }
        }
        catch( Poco::Exception ex )
        {
            completeConfigAccess();
            std::cout << "putDeviceConfig exception: " << ex.displayText() << std::endl;
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            return;
        }

        // Finish with update
        completeConfigAccess();
    }
    else if( "getDeviceOwner" == opID )
    {
        // Create a json root object
        pjs::Object jsRoot;

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        jsRoot.set( "isAvailable", m_available );
        jsRoot.set( "isOwned", m_owned );

        if( m_owned == true )
        {
          jsRoot.set( "owner_hnodeID", getOwnerHNodeIDStr() );
          jsRoot.set( "owner_crc32ID", getOwnerCRC32IDStr() );
        }

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
    else if( "putDeviceOwner" == opID )
    {
        std::istream& bodyStream = opData->requestBody();

        // Prepare for object update
        startConfigAccess();

        // Parse the json body of the request
        try
        {
            // Attempt to parse the json
            pjs::Parser parser;
            pdy::Var varRoot = parser.parse( bodyStream );

            // Get a pointer to the root object
            pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

            if( jsRoot->has( "owner_hnodeID" ) )
            {
                setOwner( jsRoot->getValue<std::string>( "owner_hnodeID" ) );
            }
        }
        catch( Poco::Exception ex )
        {
            completeConfigAccess();
            std::cout << "putDeviceOwner exception: " << ex.displayText() << std::endl;
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            return;
        }

        // Finish with update
        completeConfigAccess();
    }
    else if( "deleteDeviceOwner" == opID )
    {
        // Clear the current owner
        startConfigAccess();
        clearOwner();
        completeConfigAccess();
    }
    else if( "getDeviceHealth" == opID )
    {
        HNDH_RESULT_T result;

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        // Render the response
        std::ostream& ostr = opData->responseSend();
        result = m_health.getRestJSON( ostr );

        if( result != HNDH_RESULT_SUCCESS )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            return;
        }
    }
    else if( "getDeviceHealthComponent" == opID )
    {
        // FIXME Send back not implemented
        opData->responseSetStatusAndReason( HNR_HTTP_NOT_IMPLEMENTED );
        return;
    }
    else
    {
        // Send back not implemented
        opData->responseSetStatusAndReason( HNR_HTTP_NOT_IMPLEMENTED );
        return;
    }

    // Send back ok
    opData->responseSetStatusAndReason( HNR_HTTP_OK );
    opData->responseSend();
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
      },
      "put": {
        "summary": "Update user configurable device settings, i.e. name",
        "operationId": "putDeviceConfig",
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
            "description": "Invalid request"
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
      },
      "put": {
        "summary": "Claim/Update ownership of device.",
        "operationId": "putDeviceOwner",
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
      },
      "delete": {
        "summary": "Release ownership of device.",
        "operationId": "deleteDeviceOwner",
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

    "/hnode2/device/health": {
      "get": {
        "summary": "Get health monitoring status for this device.",
        "operationId": "getDeviceHealth",
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

    "/hnode2/device/health/{compID}": {
      "get": {
        "summary": "Get health monitoring status for a single component.",
        "operationId": "getDeviceHealthComponent",
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
