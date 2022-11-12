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

HNDServiceRecord::HNDServiceRecord()
{
    m_specType = HNDS_TYPE_NOTSET;
    m_isMapped = false;
}

HNDServiceRecord::~HNDServiceRecord()
{

}

void
HNDServiceRecord::setSpecType( HNDS_TYPE_T stype )
{
    m_specType = stype;
}

void
HNDServiceRecord::setType( std::string value )
{
    m_svcType = value;
}

void
HNDServiceRecord::setVersion( std::string value )
{
    m_version = value;
}
        
void
HNDServiceRecord::setCustomURIFromStr( std::string uri )
{
    m_customURI = uri;
}

void
HNDServiceRecord::setExtPath( std::string path )
{
    m_extPath = path;
}

#if 0
void
HNDServiceRecord::setDeviceExtension( bool value )
{
    m_isDeviceExtension = value;
}
#endif

void
HNDServiceRecord::setMapped( bool value )
{
    m_isMapped = value;
}

void
HNDServiceRecord::setProviderCRC32ID( std::string crc32ID )
{
    m_providerCRC32ID = crc32ID;
}

void
HNDServiceRecord::setProviderURIFromStr( std::string uri )
{
    m_providerURI = uri;
}

HNDS_TYPE_T
HNDServiceRecord::getSpecType()
{
    return m_specType;
}    

std::string
HNDServiceRecord::getVersion()
{
    return m_version;
}

std::string
HNDServiceRecord::getCustomURIAsStr()
{
    return m_customURI;
}

std::string
HNDServiceRecord::getExtPath()
{
    return m_extPath;
}

std::string
HNDServiceRecord::getType()
{
    return m_svcType;
}

#if 0
bool
HNDServiceRecord::isDeviceExtension()
{
    return m_isDeviceExtension;
}
#endif

bool
HNDServiceRecord::isMapped()
{
    return m_isMapped;
}

std::string
HNDServiceRecord::getProviderCRC32ID()
{
    return m_providerCRC32ID;
}

std::string
HNDServiceRecord::getProviderURIAsStr()
{
    return m_providerURI;
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
    m_rootPath    = "hnode2";

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
    m_rootPath    = "hnode2";

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
HNodeDevice::setRestAddress( std::string addrStr )
{

}

void 
HNodeDevice::setRestPort( uint16_t port )
{
    m_port = port;
}

void 
HNodeDevice::setRestRootPath( std::string pathStr )
{
    m_rootPath = pathStr;
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

uint32_t
HNodeDevice::getHNodeIDCRC32()
{
    return m_hnodeID.getCRC32();
}

std::string
HNodeDevice::getName()
{
    return m_name;
}

std::string
HNodeDevice::getRestAddress()
{
    if( m_address.empty() == true )
    {
        return m_network.getDefaultIPV4AddrStr();
    }

    return m_address;
}

uint16_t
HNodeDevice::getRestPort()
{
    return m_port;
}

std::string
HNodeDevice::getRestRootPath()
{
    return m_rootPath;
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

bool
HNodeDevice::hasHealthMonitoring()
{
    return m_health.isEnabled();
}

bool
HNodeDevice::hasEventing()
{
    return false;
}

bool
HNodeDevice::hasLogging()
{
    return false;
}

bool
HNodeDevice::hasDataCollection()
{
    return false;
}

bool
HNodeDevice::hasKeyValue()
{
    return false;
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

void
HNodeDevice::clearProvidedServices()
{
    m_advertisedServices.clear();
}

void
HNodeDevice::clearProvidedService( std::string typeStr )
{
    std::map< std::string, HNDServiceRecord >::iterator it = m_advertisedServices.find( typeStr );

    if( it != m_advertisedServices.end() )
        m_advertisedServices.erase( it );
}

void
HNodeDevice::registerProvidedServiceCustom( std::string typeStr, std::string versionStr, std::string uri )
{
    HNDServiceRecord srvRec;

    srvRec.setSpecType( HNDS_TYPE_CUSTOM );

    srvRec.setType( typeStr );
    srvRec.setVersion( versionStr );
    srvRec.setCustomURIFromStr( uri );

    m_advertisedServices.insert( std::pair<std::string, HNDServiceRecord>( typeStr, srvRec ) );
}

void
HNodeDevice::registerProvidedServiceExtension( std::string typeStr, std::string versionStr, std::string extPath )
{
    HNDServiceRecord srvRec;

    srvRec.setSpecType( HNDS_TYPE_DEVICEREST );

    srvRec.setType( typeStr );
    srvRec.setVersion( versionStr );
    srvRec.setExtPath( extPath );

    m_advertisedServices.insert( std::pair<std::string, HNDServiceRecord>( typeStr, srvRec ) );
}

void
HNodeDevice::clearDesiredServices()
{
    m_desiredServices.clear();
}

void
HNodeDevice::clearDesiredService( std::string typeStr )
{
    std::map< std::string, HNDServiceRecord >::iterator it = m_desiredServices.find( typeStr );

    if( it != m_desiredServices.end() )
        m_desiredServices.erase( it );
}

void
HNodeDevice::addDesiredService( std::string typeStr )
{
    HNDServiceRecord srvRec;

    srvRec.setType( typeStr );

    m_desiredServices.insert( std::pair<std::string, HNDServiceRecord>( typeStr, srvRec ) );
}

bool
HNodeDevice::isServiceMapped( std::string typeStr )
{
    std::map< std::string, HNDServiceRecord >::iterator it = m_desiredServices.find( typeStr );

    if( it != m_desiredServices.end() )
        return false;

    return it->second.isMapped();
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

    // Refresh the networking data for the current node
    m_network.refreshData();

    m_network.debugPrint();

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

    // Setup device REST
    m_rest.setPort( m_port );

    // Register the devices root REST interface
    // std::string devURI = m_rest.formatURI( m_network.getDefaultIPV4AddrStr(), "device" );
    registerProvidedServiceExtension( "hnsrv-hnode2-device", "1.0.0", "device" );

    // std::cout << "Default HNode URI: " << devURI << std::endl;

    std::cout << "Started HNAvahi..." << std::endl;

    // Indicate support for healh information requests
    if( m_health.isEnabled() == true )
    {
        m_health.init( createAvahiName(), m_hnodeID.getCRC32AsHexStr(), getName() );

        // Indicate support for healh information requests
        registerProvidedServiceExtension( "hnsrv-health-source", "1.0.0", "device/health" );

        // Indicate support for sending health information
        addDesiredService( "hnsrv-health-sink" );
    }

    // Start serving requests
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
            opData->responseSend();
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
            opData->responseSend();
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
            opData->responseSend();
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
            opData->responseSend();
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
    // GET /hnode2/device/services
    else if( "getDeviceServices" == opID )
    {
        // Create a json root object
        pjs::Object jsRoot;
        pjs::Array  jsProvidedServices;
        pjs::Array  jsDesiredServices;

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        for( std::map<std::string, HNDServiceRecord>::iterator it = m_advertisedServices.begin(); it != m_advertisedServices.end(); it++ )
        {        
            pjs::Object jsService;

            jsService.set( "type", it->second.getType() );
            jsService.set( "version", it->second.getVersion() );

            std::string uriStr;
            switch( it->second.getSpecType() )
            {
                case HNDS_TYPE_DEVICEREST:
                {
                    // FIXME to handle address override cases, etc.
                    uriStr = m_rest.formatURI( m_network.getDefaultIPV4AddrStr(), it->second.getExtPath() );
                }
                break;

                case HNDS_TYPE_CUSTOM:
                {
                    uriStr = it->second.getCustomURIAsStr();
                }
                break;

                // Mal-formed service record, move on.
                default:
                continue;
            }
            jsService.set( "uri", uriStr );

            jsProvidedServices.add( jsService );
        }

        jsRoot.set( "providedServices", jsProvidedServices );

        for( std::map<std::string, HNDServiceRecord>::iterator it = m_desiredServices.begin(); it != m_desiredServices.end(); it++ )
        {        
            pjs::Object jsService;

            jsService.set( "type", it->second.getType() );
            jsService.set( "version", it->second.getVersion() );
            jsService.set( "mapped-uri", it->second.getCustomURIAsStr() );

            jsDesiredServices.add( jsService );
        }

        jsRoot.set( "desiredServices", jsDesiredServices );

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
            opData->responseSend();
            return;
        }
    }
    // GET /hnode2/device/services/provided
    else if( "getServicesProvidedList" == opID )
    {
        // Create a json root object
        pjs::Array  jsProvidedServices;

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        for( std::map<std::string, HNDServiceRecord>::iterator it = m_advertisedServices.begin(); it != m_advertisedServices.end(); it++ )
        {        
            pjs::Object jsService;

            jsService.set( "type", it->second.getType() );
            jsService.set( "version", it->second.getVersion() );

            std::string uriStr;
            switch( it->second.getSpecType() )
            {
                case HNDS_TYPE_DEVICEREST:
                {
                    // FIXME to handle address override cases, etc.
                    uriStr = m_rest.formatURI( m_network.getDefaultIPV4AddrStr(), it->second.getExtPath() );
                }
                break;

                case HNDS_TYPE_CUSTOM:
                {
                    uriStr = it->second.getCustomURIAsStr();
                }
                break;

                // Mal-formed service record, move on.
                default:
                continue;
            }
            jsService.set( "uri", uriStr );

            jsProvidedServices.add( jsService );
        }

        // Render the response
        std::ostream& ostr = opData->responseSend();
        try
        {
            // Write out the generated json
            pjs::Stringifier::stringify( jsProvidedServices, ostr, 1 );
        }
        catch( ... )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return;
        }
    }
    // GET /hnode2/device/services/provided/{serviceType}
    else if( "getServicesProvided" == opID )
    {
        HNDH_RESULT_T result;
        std::string serviceType;

        if( opData->getParam( "serviceType", serviceType ) == true )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return; 
        }

        // Create a json root object
        pjs::Object jsService;

        std::map<std::string, HNDServiceRecord>::iterator it = m_advertisedServices.find( serviceType );

        if( it == m_advertisedServices.end() )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_NOT_FOUND );
            opData->responseSend();
            return; 
        }

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        jsService.set( "type", it->second.getType() );
        jsService.set( "version", it->second.getVersion() );

        std::string uriStr;
        switch( it->second.getSpecType() )
        {
            case HNDS_TYPE_DEVICEREST:
            {
                // FIXME to handle address override cases, etc.
                uriStr = m_rest.formatURI( m_network.getDefaultIPV4AddrStr(), it->second.getExtPath() );
            }
            break;

            case HNDS_TYPE_CUSTOM:
            {
                uriStr = it->second.getCustomURIAsStr();
            }
            break;

            // Mal-formed service record, move on.
            default:
            break;
        }
        jsService.set( "uri", uriStr );

        // Render the response
        std::ostream& ostr = opData->responseSend();
        try
        {
            // Write out the generated json
            pjs::Stringifier::stringify( jsService, ostr, 1 );
        }
        catch( ... )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return;
        }
    }
    // GET /hnode2/device/services/mappings
    else if( "getServiceMappings" == opID )
    {
        // Create a json root object
        pjs::Array  jsDesiredServices;

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        for( std::map<std::string, HNDServiceRecord>::iterator it = m_desiredServices.begin(); it != m_desiredServices.end(); it++ )
        {        
            pjs::Object jsService;

            jsService.set( "type", it->second.getType() );
            jsService.set( "version", it->second.getVersion() );
            jsService.set( "mapped-uri", it->second.getCustomURIAsStr() );

            jsDesiredServices.add( jsService );
        }

        // Render the response
        std::ostream& ostr = opData->responseSend();
        try
        {
            // Write out the generated json
            pjs::Stringifier::stringify( jsDesiredServices, ostr, 1 );
        }
        catch( ... )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return;
        }
    }
    // PUT /hnode2/device/services/mappings
    else if( "putServiceMappings" == opID )
    {
        HNDH_RESULT_T result;

        // Parse the json body of the request
        std::istream& bodyStream = opData->requestBody();
        try
        {
            // Attempt to parse the json    
            pjs::Parser parser;
            pdy::Var varRoot = parser.parse( bodyStream );

            if( varRoot.isArray() == false )
            {
                opData->responseSetStatusAndReason( HNR_HTTP_BAD_REQUEST );
                opData->responseSend();
                return;
            }

            // Get a pointer to the root object
            pjs::Array::Ptr jsRoot = varRoot.extract< pjs::Array::Ptr >();

            pjs::Stringifier::stringify( jsRoot, std::cout, 1 );

            // Go through each object in the array
            for( uint i = 0; i < jsRoot->size(); i++ )
            {
                // Make sure array member is an object
                if( jsRoot->isObject( i ) == false )
                    continue;

                pjs::Object::Ptr jsService = jsRoot->getObject( i );

                if( jsService->has( "type" ) == false )
                    continue;

                std::string srvType = jsService->getValue<std::string>( "type" );

                std::map< std::string, HNDServiceRecord >::iterator it = m_desiredServices.find( srvType );

                if( it == m_desiredServices.end() )
                    continue;

                if( jsService->has( "mapped-uri" ) )
                {
                    std::cout << "Mapping service: " << srvType << "  to: " << jsService->getValue<std::string>( "mapped-uri" ) << std::endl;

                    it->second.setCustomURIFromStr( jsService->getValue<std::string>( "mapped-uri" ) );
                    it->second.setMapped( true );
                }
            }

            // m_health.setServiceRootURIFromStr( jsSvcObj->getValue< std::string >( "rootURI" ) );

        }
        catch( Poco::Exception ex )
        {
            std::cout << "putServiceSettings: " << ex.displayText() << std::endl;
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return; 
        }
    }  
    // GET /hnode2/device/services/mappings/{serviceType}
    else if( "getServiceMapping" == opID )
    {
        HNDH_RESULT_T result;
        std::string serviceType;

        if( opData->getParam( "serviceType", serviceType ) == true )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return; 
        }

        std::map< std::string, HNDServiceRecord >::iterator it = m_desiredServices.find( serviceType );

        if( it == m_desiredServices.end() )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_NOT_FOUND );
            opData->responseSend();
            return;
        }

        opData->responseSetChunkedTransferEncoding(true);
        opData->responseSetContentType("application/json");

        pjs::Object jsService;

        jsService.set( "type", it->second.getType() );
        jsService.set( "version", it->second.getVersion() );
        jsService.set( "mapped-uri", it->second.getCustomURIAsStr() );

        // Render the response
        std::ostream& ostr = opData->responseSend();
        try
        {
            // Write out the generated json
            pjs::Stringifier::stringify( jsService, ostr, 1 );
        }
        catch( ... )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return;
        }
    }
    // PUT /hnode2/device/service-mappings/{serviceType}
    else if( "putServiceMapping" == opID )
    {
        HNDH_RESULT_T result;
        std::string serviceType;

        if( opData->getParam( "serviceType", serviceType ) == true )
        {
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return; 
        }

        // Parse the json body of the request
        std::istream& bodyStream = opData->requestBody();
        try
        {
            // Attempt to parse the json    
            pjs::Parser parser;
            pdy::Var varRoot = parser.parse( bodyStream );

            // Get a pointer to the root object
            pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();
// FIXME
#if 0
                if( jsRoot->has( "rootURI" ) )
                {
                    m_health.setServiceRootURIFromStr( jsRoot->getValue< std::string >( "rootURI" ) );
                }
                else
                {
                    m_health.clearService();
                }
#endif          
        }
        catch( Poco::Exception ex )
        {
            std::cout << "putSingleServiceSettings: " << ex.displayText() << std::endl;
            opData->responseSetStatusAndReason( HNR_HTTP_INTERNAL_SERVER_ERROR );
            opData->responseSend();
            return; 
        }
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
            opData->responseSend();
            return;
        }
    }
    else if( "getDeviceHealthComponent" == opID )
    {
        // FIXME Send back not implemented
        opData->responseSetStatusAndReason( HNR_HTTP_NOT_IMPLEMENTED );
        opData->responseSend();
        return;
    }    
    else
    {
        // Send back not implemented
        opData->responseSetStatusAndReason( HNR_HTTP_NOT_IMPLEMENTED );
        opData->responseSend();
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
    
    "/hnode2/device/services": {
      "get": {
        "summary": "Get info about services provided and services desired by this node",
        "operationId": "getDeviceServices",
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

    "/hnode2/device/services/provided": {
      "get": {
        "summary": "Get information about all provided service-endpoints",
        "operationId": "getServicesProvidedList",
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

    "/hnode2/device/services/provided/{serviceType}": {
      "get": {
        "summary": "Get information for a specific service endpoint.",
        "operationId": "getServicesProvided",
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

    "/hnode2/device/services/mappings": {
      "get": {
        "summary": "Get info about current mappings for desired services.",
        "operationId": "getServiceMappings",
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
        "summary": "Bulk modify desired services mappings.",
        "operationId": "putServiceMappings",
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

    "/hnode2/device/services/mappings/{serviceType}": {
      "get": {
        "summary": "Get info about a single service's settings.",
        "operationId": "getServiceMapping",
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
        "summary": "Modify a single service mapping.",
        "operationId": "putServiceMapping",
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
    }
  }
}
)";
