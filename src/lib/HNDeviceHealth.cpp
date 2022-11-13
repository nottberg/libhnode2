#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <mutex>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Checksum.h>

#include "HNDeviceHealth.h"

namespace pjs = Poco::JSON;
namespace pdy = Poco::Dynamic;

/*
HNDHFormatStr::HNDHFormatStr( uint code, std::string formatStr )
{
    m_code = code;
    m_formatStr = formatStr;
}

HNDHFormatStr::~HNDHFormatStr()
{

}
*/

HNDHComponent::HNDHComponent( std::string compID, std::string componentName )
{
    m_compID = compID;
    m_compName = componentName;

    m_stdStatus = HNDH_CSTAT_UNKNOWN;
    m_propagatedStatus = HNDH_CSTAT_UNKNOWN;

    m_errCode  = 0;
}

HNDHComponent::~HNDHComponent()
{

}

void
HNDHComponent::clear()
{
    m_compID.clear();
    m_compName.clear();
    m_stdStatus = HNDH_CSTAT_UNKNOWN;
    m_propagatedStatus = HNDH_CSTAT_UNKNOWN;
    m_errCode = 0;
    m_parentID.clear();
    m_msgInstance.clear();
    m_noteInstance.clear();
}

void
HNDHComponent::setID( std::string idStr )
{
    m_compID = idStr;
}

void
HNDHComponent::setName( std::string compName )
{
    m_compName = compName;
}

void 
HNDHComponent::setStatus( HNDH_CSTAT_T status )
{
    m_stdStatus = status;
}

void 
HNDHComponent::setPropagatedStatus( HNDH_CSTAT_T status )
{
    m_propagatedStatus = status;
}

void 
HNDHComponent::setErrorCode( unsigned int code )
{
    m_errCode = code;
}

void
HNDHComponent::setParentID( std::string parentID )
{
    m_parentID = parentID;
}

void
HNDHComponent::setUpdateTimestamp( time_t updateTS )
{
    m_lastUpdateTS = updateTS;
}

std::string
HNDHComponent::getID()
{
    return m_compID;
}

std::string
HNDHComponent::getComponentName()
{
    return m_compName;
}

HNDH_CSTAT_T
HNDHComponent::getStatus()
{
    return m_stdStatus;
}

std::string
HNDHComponent::getStatusAsStr()
{
    switch( m_stdStatus )
    {
        case HNDH_CSTAT_UNKNOWN:
            return "UNKNOWN";
        break;

        case HNDH_CSTAT_OK:
            return "OK";
        break;

        case HNDH_CSTAT_DEGRADED:
            return "DEGRADED";
        break;
    }

    return "FAILED";
}

HNDH_CSTAT_T
HNDHComponent::getPropagatedStatus()
{
    return m_propagatedStatus;
}

std::string
HNDHComponent::getPropagatedStatusAsStr()
{
    switch( m_propagatedStatus )
    {
        case HNDH_CSTAT_UNKNOWN:
            return "UNKNOWN";
        break;

        case HNDH_CSTAT_OK:
            return "OK";
        break;

        case HNDH_CSTAT_DEGRADED:
            return "DEGRADED";
        break;
    }

    return "FAILED";
}

time_t
HNDHComponent::getLastUpdateTime()
{
    return m_lastUpdateTS;
}

std::string
HNDHComponent::getLastUpdateTimeAsStr()
{
    char tmpBuf[128];
    ctime_r( &m_lastUpdateTS, tmpBuf );
    return tmpBuf;
}

uint
HNDHComponent::getErrorCode()
{
    return m_errCode;
}
        
std::string
HNDHComponent::getRenderedMsg()
{
    return m_msgInstance.getResultStr();
}

std::string
HNDHComponent::getRenderedNote()
{
    return m_noteInstance.getResultStr();
}

void
HNDHComponent::clearMsgInstance()
{
    m_msgInstance.clear();
}

HNFSInstance&
HNDHComponent::getMsgInstanceRef()
{
    return m_msgInstance;
}

void
HNDHComponent::clearNoteInstance()
{
    m_noteInstance.clear();
}

HNFSInstance&
HNDHComponent::getNoteInstanceRef()
{
    return m_noteInstance;
}

void 
HNDHComponent::addChildComponent( HNDHComponent *childComp )
{
    m_children.push_back( childComp );
}

std::vector< HNDHComponent* >& 
HNDHComponent::getChildListRef()
{
    return m_children;
}

HNDeviceHealth::HNDeviceHealth( HNFormatStringStore *stringStore, HNHttpEventClient *evClient )
: m_devStatus( HNDH_ROOT_COMPID, "Device Health" )
{
    m_enabled = false;

    m_lockedForUpdate = false;

    m_stringStore = stringStore;

    m_evClient = evClient;

    std::cout << "Root ID: " << m_devStatus.getID() << std::endl;

    m_compStatus.insert( std::pair< std::string, HNDHComponent* >( m_devStatus.getID(), &m_devStatus ) );
}

HNDeviceHealth::~HNDeviceHealth()
{

}

void 
HNDeviceHealth::setEnabled()
{
    m_enabled = true;
}

bool 
HNDeviceHealth::isEnabled()
{
    return m_enabled;
}

void 
HNDeviceHealth::clear()
{
    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_accessMutex );

    m_lockedForUpdate = false;

    m_compStatus.clear();
    
    m_devStatus.clear();
    m_devStatus.setID( HNDH_ROOT_COMPID );
    m_devStatus.setName( "Device Overall Health" );

    std::cout << "Root ID 2: " << m_devStatus.getID() << std::endl;

    m_compStatus.insert( std::pair< std::string, HNDHComponent* >( m_devStatus.getID(), &m_devStatus ) );

    m_enabled = false;
}

HNDH_RESULT_T 
HNDeviceHealth::allocUniqueID( std::string &compID )
{
    char tmpID[ 64 ];
    uint idNum = 1;

    compID.clear();

    // Allocate a new component ID and add this component.
    idNum = m_compStatus.size();
    if( idNum == 0 )
        idNum = 1;

    while( idNum < 2000 )
    {     
        sprintf( tmpID, "c%d", idNum );

        if( m_compStatus.find( tmpID ) == m_compStatus.end() )
        {
            compID = tmpID;
            return HNDH_RESULT_SUCCESS;
        }

        idNum += 1;
    }

    return HNDH_RESULT_FAILURE;
}

HNDH_RESULT_T
HNDeviceHealth::init( std::string deviceID, std::string deviceCRC32, std::string deviceName )
{
    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_accessMutex );

    m_deviceID    = deviceID;
    m_deviceCRC32 = deviceCRC32;

    m_devStatus.setName( deviceName );
    m_devStatus.setStatus( HNDH_CSTAT_UNKNOWN );

    return HNDH_RESULT_SUCCESS;
}

HNDH_RESULT_T 
HNDeviceHealth::registerComponent( std::string componentName, std::string parentID, std::string &compID )
{
    HNDHComponent *parent;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_accessMutex );

    // If there is a parentID then ensure it already exists in the list
    std::map<std::string, HNDHComponent* >::iterator it;

    // Look for an existing entry 
    it = m_compStatus.find( parentID );

    // If couldn't find referenced parent then fail.
    if( it == m_compStatus.end() )
    {
        std::cout << "Failed to find parent: " << parentID << std::endl;
        return HNDH_RESULT_FAILURE;
    }

    parent = it->second;

    // Allocate a new component ID and add this component.
    if( allocUniqueID( compID ) != HNDH_RESULT_SUCCESS )
        return HNDH_RESULT_FAILURE;

    HNDHComponent *newComp = new HNDHComponent( compID, componentName );

    newComp->setParentID( parent->getID() );

    m_compStatus.insert( std::pair<std::string, HNDHComponent*>( compID, newComp ) );

    parent->addChildComponent( newComp );

    return HNDH_RESULT_SUCCESS;
}

// Start a new update cycle
void
HNDeviceHealth::startUpdateCycle( time_t updateTimestamp )
{
    // Look the access mutex, which will be unlocked with a mandatory
    // call to completeUpdateCycle() at the end of the update process.
    m_accessMutex.lock();

    // Allow status updates    
    m_lockedForUpdate = true;

    // Record the update cycle timestamp
    m_updateCycleTimestamp = updateTimestamp;

    // Track status change
    m_statusChange = false;
}

// Complete an update cycle, return whether values changed.
bool
HNDeviceHealth::completeUpdateCycle()
{
    // Propogate any negative status up the parent chain.
    if( propagateStatus() == true )
    {
        m_statusChange = true;
    }

    // Don't allow status updates
    m_lockedForUpdate = false;

    // If the status changed, and the sink is setup then generate a change event
    if( ( m_statusChange == true ) && ( m_sinkURI.empty() == false ) && ( m_evClient != NULL ) )
    {
        pjs::Object jsRoot;

        jsRoot.set( "devID", m_deviceCRC32 );
        jsRoot.set( "devStatus", m_devStatus.getPropagatedStatusAsStr() );

        std::stringstream payload;
        try {
            pjs::Stringifier::stringify( jsRoot, payload, 1 );
        } catch( ... ) {
        }

        std::string evURI = m_sinkURI + "/event";
        m_evClient->submitPostEvent( evURI, "application/json", payload.str() );
    }

    // Release the access mutex since the update operation 
    // has completed.
    m_accessMutex.unlock();

    // Return whether updates resulted in a status change.
    return m_statusChange;
}

void
HNDeviceHealth::setComponentStatus( std::string compID, HNDH_CSTAT_T stdStatus )
{
    HNDHComponent *comp;

    // Make sure we are in an update cycle.
    if( m_lockedForUpdate ==  false )
    {
        std::cerr << "ERROR: Health Component Updates must be surrounded by startUpdate, completeUpdate calls." << std::endl;
        return;
    }

    // Look for an existing entry 
    std::map<std::string, HNDHComponent* >::iterator it = m_compStatus.find( compID );

    // If couldn't find referenced parent then fail.
    if( it == m_compStatus.end() )
        return;

    comp = it->second;

    if( comp->getStatus() != stdStatus )
    {
        m_statusChange = true;
        comp->setStatus( stdStatus );
    }

    comp->setUpdateTimestamp( m_updateCycleTimestamp );
}

void
HNDeviceHealth::clearComponentErrMsg( std::string compID )
{
    HNDHComponent *comp;

    // Make sure we are in an update cycle.
    if( m_lockedForUpdate ==  false )
    {
        std::cerr << "ERROR: Health Component Updates must be surrounded by startUpdate, completeUpdate calls." << std::endl;
        return;
    }

    // Look for an existing entry 
    std::map<std::string, HNDHComponent* >::iterator it = m_compStatus.find( compID );

    // If couldn't find referenced parent then fail.
    if( it == m_compStatus.end() )
        return;

    comp = it->second;

    if( comp->getErrorCode() != 0 )
    {
        m_statusChange = true;
        comp->setErrorCode( 0 );
    }

    if( comp->getMsgInstanceRef().clear() == true )
    {
        m_statusChange = true;
    }

    comp->setUpdateTimestamp( m_updateCycleTimestamp );
}

void
HNDeviceHealth::setComponentErrMsg( std::string compID, uint errCode, uint fmtCode, ... )
{
    HNDHComponent *comp;
    va_list vargs;

    // Make sure we are in an update cycle.
    if( m_lockedForUpdate ==  false )
    {
        std::cerr << "ERROR: Health Component Updates must be surrounded by startUpdate, completeUpdate calls." << std::endl;
        return;
    }

    // Look for an existing entry 
    std::map<std::string, HNDHComponent* >::iterator it = m_compStatus.find( compID );

    // Verify string store is available.
    if( m_stringStore == NULL )
        return;

    // If couldn't find referenced parent then fail.
    if( it == m_compStatus.end() )
        return;

    comp = it->second;

    // Get access to the variable parameter list
    va_start( vargs, fmtCode );

    if( comp->getErrorCode() != errCode )
    {
        m_statusChange = true;
        comp->setErrorCode( errCode );
    }

    if( m_stringStore->fillInstance( fmtCode, vargs, comp->getMsgInstanceRef() ) == HNFS_RESULT_SUCCESS_CHANGED )
    {
        m_statusChange = true;
    }

    va_end( vargs );

    comp->setUpdateTimestamp( m_updateCycleTimestamp );
}

void
HNDeviceHealth::clearComponentNote( std::string compID )
{
    HNDHComponent *comp;

    // Make sure we are in an update cycle.
    if( m_lockedForUpdate ==  false )
    {
        std::cerr << "ERROR: Health Component Updates must be surrounded by startUpdate, completeUpdate calls." << std::endl;
        return;
    }

    // Look for an existing entry 
    std::map<std::string, HNDHComponent* >::iterator it = m_compStatus.find( compID );

    // If couldn't find referenced parent then fail.
    if( it == m_compStatus.end() )
        return;

    comp = it->second;

    if( comp->getNoteInstanceRef().clear() == true )
    {
        m_statusChange = true;
    }

    comp->setUpdateTimestamp( m_updateCycleTimestamp );
}

void 
HNDeviceHealth::setComponentNote( std::string compID, uint fmtCode, ... )
{
    HNDHComponent *comp;
    va_list vargs;

    // Make sure we are in an update cycle.
    if( m_lockedForUpdate ==  false )
    {
        std::cerr << "ERROR: Health Component Updates must be surrounded by startUpdate, completeUpdate calls." << std::endl;
        return;
    }

    // Look for an existing entry 
    std::map<std::string, HNDHComponent* >::iterator it = m_compStatus.find( compID );

    // Verify string store is available.
    if( m_stringStore == NULL )
        return;

    // If couldn't find referenced parent then fail.
    if( it == m_compStatus.end() )
        return;

    comp = it->second;

    // Get access to the variable parameter list
    va_start( vargs, fmtCode );

    if( m_stringStore->fillInstance( fmtCode, vargs, comp->getNoteInstanceRef() ) == HNFS_RESULT_SUCCESS_CHANGED )
    {
        m_statusChange = true;
    }

    va_end( vargs );

    comp->setUpdateTimestamp( m_updateCycleTimestamp );
}

HNDH_CSTAT_T
HNDeviceHealth::propagateChild( HNDHComponent *comp, bool &changed )
{
    HNDH_CSTAT_T calcStatus = comp->getStatus();

    // Do a depth first walk of the tree
    for( std::vector< HNDHComponent* >::iterator it = comp->getChildListRef().begin(); 
            it != comp->getChildListRef().end(); 
                it++ )
    {
        HNDH_CSTAT_T childStatus = propagateChild( *it, changed );

        switch( childStatus )
        {
            case HNDH_CSTAT_UNKNOWN:
            case HNDH_CSTAT_DEGRADED:
            case HNDH_CSTAT_FAILED:
                calcStatus = HNDH_CSTAT_DEGRADED;
            break;

            case HNDH_CSTAT_OK:
            break;
        }
    }
    
    std::cout << "setStat: " << comp->getStatus() << "  propStat: " << comp->getPropagatedStatus() << "  calcStat:" << calcStatus << std::endl;

    // Set the propogated status for the current child.
    if( comp->getPropagatedStatus() != calcStatus )
    {
        comp->setPropagatedStatus( calcStatus );
        changed = true;
    }

    return calcStatus;
}

bool
HNDeviceHealth::propagateStatus()
{
    bool changed = false;

    propagateChild( &m_devStatus, changed );

    return changed;
}

HNDH_RESULT_T
HNDeviceHealth::addCompJSONObject( void *listPtr, HNDHComponent *comp )
{
    pjs::Array *jsCompList = (pjs::Array *) listPtr;
    pjs::Object jsComp;

    jsComp.set( "id", comp->getID() );
    jsComp.set( "name", comp->getComponentName() );
    jsComp.set( "setStatus", comp->getStatusAsStr() );
    jsComp.set( "propagatedStatus", comp->getPropagatedStatusAsStr() );
    jsComp.set( "errCode", comp->getErrorCode() );
    jsComp.set( "msgStr", comp->getRenderedMsg() );
    jsComp.set( "noteStr", comp->getRenderedNote() );
    jsComp.set( "updateTime", comp->getLastUpdateTimeAsStr() );

    pjs::Array jsChildList;
    for( std::vector< HNDHComponent* >::iterator it = comp->getChildListRef().begin(); it != comp->getChildListRef().end(); it++ )
    {
        addCompJSONObject( &jsChildList, *it );
    }
    jsComp.set( "children", jsChildList );

    jsCompList->add( jsComp );

    return HNDH_RESULT_SUCCESS;
}

HNDH_RESULT_T 
HNDeviceHealth::getRestJSON( std::string &jsonStr )
{
    HNDH_RESULT_T result;
    std::ostringstream ostr;

    jsonStr.clear();

    result = getRestJSON( ostr );

    if( result != HNDH_RESULT_SUCCESS )
        return result;

    jsonStr = ostr.str();

    return HNDH_RESULT_SUCCESS;
}

HNDH_RESULT_T 
HNDeviceHealth::getRestJSON( std::ostream &oStream )
{
    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_accessMutex );

    // Create a json root object
    pjs::Object jsRoot;
    
    jsRoot.set( "enabled", m_enabled );

    if( m_enabled == true )
    {
        jsRoot.set( "deviceStatus", m_devStatus.getPropagatedStatusAsStr() );
        jsRoot.set( "deviceID", m_deviceID );
        jsRoot.set( "deviceCRC32", m_deviceCRC32 );

        pjs::Array jsCompList;
        addCompJSONObject( &jsCompList, &m_devStatus );

        jsRoot.set( "components", jsCompList );
    }

    // Render the response
    try
    {
        // Write out the generated json
        pjs::Stringifier::stringify( jsRoot, oStream, 1 );
    }
    catch( ... )
    {
        return HNDH_RESULT_FAILURE;
    }

    return HNDH_RESULT_SUCCESS;
}

void
HNDeviceHealth::clearSinkMapping()
{
    m_sinkURI.clear();
}

void
HNDeviceHealth::checkSinkMapping( std::string uri )
{
    std::cout << "HNDeviceHealth::checkSinkMapping - uri: " << uri << std::endl;
    m_sinkURI = uri;
}

std::string
HNDeviceHealth::getSinkMapping()
{
    return m_sinkURI;
}
