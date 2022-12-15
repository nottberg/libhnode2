#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <iomanip>

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

HNDHComponent::HNDHComponent( std::string compID )
{
    m_compID = compID;

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
    m_stdStatus = HNDH_CSTAT_UNKNOWN;
    m_propagatedStatus = HNDH_CSTAT_UNKNOWN;
    m_errCode = 0;
    m_parentID.clear();

    m_nameInstance.clear();
    m_descInstance.clear();
    m_msgInstance.clear();
    m_noteInstance.clear();
}

void
HNDHComponent::setID( std::string idStr )
{
    m_compID = idStr;
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

HNDH_CSTAT_T
HNDHComponent::getStatus()
{
    return m_stdStatus;
}

void
HNDHComponent::setStatusFromStr( std::string status )
{
    if( status == "OK" )
        m_stdStatus = HNDH_CSTAT_OK;
    else if( status == "DEGRADED")
        m_stdStatus = HNDH_CSTAT_DEGRADED;
    else if( status == "FAILED" )
        m_stdStatus = HNDH_CSTAT_FAILED;
    else
        m_stdStatus = HNDH_CSTAT_UNKNOWN;
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

void
HNDHComponent::setPropagatedStatusFromStr( std::string status )
{
    if( status == "OK" )
        m_propagatedStatus = HNDH_CSTAT_OK;
    else if( status == "DEGRADED")
        m_propagatedStatus = HNDH_CSTAT_DEGRADED;
    else if( status == "FAILED" )
        m_propagatedStatus = HNDH_CSTAT_FAILED;
    else
        m_propagatedStatus = HNDH_CSTAT_UNKNOWN;
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

/*
void
HNDHComponent::setUpdateTimestampFromStr( std::string updateTS )
{
   struct tm *timeptr,result;

   setlocale(LC_ALL,"/QSYS.LIB/EN_US.LOCALE");
   t = time(NULL);
   timeptr = localtime(&t);
   strftime(buf,sizeof(buf), "%a %m/%d/%Y %r", timeptr);

   strptime( updateTS.c_str(), "%a %b %d %H:%M:%S %Y", &m_lastUpdateTS );
}
*/

uint
HNDHComponent::getErrorCode()
{
    return m_errCode;
}

/*
std::string
HNDHComponent::getRenderedName()
{
    return m_nameInstance.getResultStr();
}

std::string
HNDHComponent::getRenderedDesc()
{
    return m_descInstance.getResultStr();
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
*/

void
HNDHComponent::clearNameInstance()
{
    m_nameInstance.clear();
}

HNFSInstance*
HNDHComponent::getNameInstancePtr()
{
    return &m_nameInstance;
}

void
HNDHComponent::clearDescInstance()
{
    m_descInstance.clear();
}

HNFSInstance*
HNDHComponent::getDescInstancePtr()
{
    return &m_descInstance;
}

void
HNDHComponent::clearMsgInstance()
{
    m_msgInstance.clear();
}

HNFSInstance*
HNDHComponent::getMsgInstancePtr()
{
    return &m_msgInstance;
}

void
HNDHComponent::clearNoteInstance()
{
    m_noteInstance.clear();
}

HNFSInstance*
HNDHComponent::getNoteInstancePtr()
{
    return &m_noteInstance;
}

HNDHComponent*
HNDHComponent::getChildComponent( std::string compID )
{
    for( std::vector< HNDHComponent* >::iterator it = m_children.begin(); it != m_children.end(); it++ )
    {
        if( (*it)->getID() == compID )
            return *it;
    }

    return NULL;
}

HNDHComponent*
HNDHComponent::getChildComponent( uint index )
{
    if( index >= m_children.size() )
        return NULL;

    return m_children[index];
}

HNDHComponent*
HNDHComponent::getOrCreateChildComponent( std::string compID, bool &childCreated )
{
    HNDHComponent *rtnComp;

    rtnComp = getChildComponent( compID );

    if( rtnComp != NULL )
    {
        childCreated = false;
        return rtnComp;
    }

    childCreated = true;

    rtnComp = HNDeviceHealth::allocateNewComponent( compID );

    rtnComp->setParentID( getID() );

    addChildComponent( rtnComp );

    return rtnComp;    
}

void 
HNDHComponent::addChildComponent( HNDHComponent *childComp )
{
    m_children.push_back( childComp );
}

void
HNDHComponent::getChildIDs( std::set< std::string > &childIDs )
{
    // Get all of the current childIDs
    for( std::vector< HNDHComponent* >::iterator it = m_children.begin(); it != m_children.end(); it++ )
    {
        childIDs.insert( (*it)->getID() );
    }
}

void
HNDHComponent::deleteChildByID( std::string compID )
{
    // Find a child id and remove it.
    for( std::vector< HNDHComponent* >::iterator it = m_children.begin(); it != m_children.end(); it++ )
    {
        if( (*it)->getID() == compID )
        {
            std::cout << "Deleting child component: " << compID << std::endl;
            HNDeviceHealth::freeComponent( *it );
            m_children.erase( it );
            return;
        } 
    }
}

std::vector< HNDHComponent* >& 
HNDHComponent::getChildListRef()
{
    return m_children;
}

void
HNDHComponent::debugPrint( uint offset, HNRenderStringIntf *renderIntf, bool printChildren )
{
    std::cout << std::endl;
    std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "id" << ": " << getID() << std::endl;
    std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "name" << ": " << renderIntf->renderInstance( getNameInstancePtr() ) << std::endl;
    //std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "desc" << ": " << getRenderedDesc() << std::endl;
    std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "status" << ": " << getStatusAsStr() << std::endl;
    std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "propagated status" << ": " << getPropagatedStatusAsStr() << std::endl;
    std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "time" << ": " << getLastUpdateTimeAsStr() << std::endl;
    //std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "note" << ": " << getRenderedNote() << std::endl;
    std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "error code" << ": " << getErrorCode() << std::endl;
    //std::cout << std::setw( offset ) << " " << std::left << std::setw( 20 ) << "error msg" << ": " << getRenderedMsg() << std::endl;

    if( printChildren == false )
        return;

    for( std::vector< HNDHComponent* >::iterator it = m_children.begin(); it != m_children.end(); it++ )
    {
        (*it)->debugPrint( offset+4, renderIntf, true );
    }
}

HNDeviceHealth::HNDeviceHealth( HNFormatStringStore *stringStore, HNHttpEventClient *evClient )
//: m_devStatus( HNDH_ROOT_COMPID )
{
    m_devStatus = NULL;

    m_enabled = false;

    m_lockedForUpdate = false;

    m_stringStore = stringStore;

    m_evClient = evClient;

    m_deviceName = "HNode Default Health Component";
}

HNDeviceHealth::~HNDeviceHealth()
{
    if( m_devStatus )
    {
        HNDeviceHealth::freeComponentTree( m_devStatus );
        m_devStatus = NULL;
    }
}

void 
HNDeviceHealth::setEnabled()
{
    if( m_enabled == false )
    {
        m_enabled = true;

        // Grab the scope lock
        const std::lock_guard< std::mutex > lock( m_accessMutex );

        m_devStatus = HNDeviceHealth::allocateNewComponent( HNDH_ROOT_COMPID );
        m_devStatus->setStatus( HNDH_CSTAT_UNKNOWN );

        uint NameSID;
        m_stringStore->registerFormatString( m_deviceName, NameSID );
        m_devStatus->getNameInstancePtr()->setFmtCode( NameSID );

        m_compStatus.insert( std::pair< std::string, HNDHComponent* >( m_devStatus->getID(), m_devStatus ) );
    }
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
    
    if( m_devStatus )
        HNDeviceHealth::freeComponentTree( m_devStatus );
        
    m_devStatus = NULL;

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
HNDeviceHealth::updateDeviceInfo( std::string deviceID, std::string deviceCRC32, std::string deviceName )
{
    std::cout << "HNDeviceHealth::init" << std::endl;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_accessMutex );

    m_deviceID    = deviceID;
    m_deviceCRC32 = deviceCRC32;
    m_deviceName  = deviceName;

    // Only allow when enabled
    if( m_enabled == false )
        return HNDH_RESULT_SUCCESS;

    uint NameSID;
    m_stringStore->registerFormatString( m_deviceName, NameSID );
    m_devStatus->getNameInstancePtr()->setFmtCode( NameSID );

    return HNDH_RESULT_SUCCESS;
}

HNDH_RESULT_T 
HNDeviceHealth::registerComponent( std::string componentName, std::string parentID, std::string &compID )
{
    HNDHComponent *parent;

    // Only allow when enabled
    if( m_enabled == false )
        return HNDH_RESULT_SUCCESS;

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

    HNDHComponent *newComp = HNDeviceHealth::allocateNewComponent( compID );

    uint NameSID;
    m_stringStore->registerFormatString( componentName, NameSID );
    newComp->getNameInstancePtr()->setFmtCode( NameSID );

    newComp->setParentID( parent->getID() );

    m_compStatus.insert( std::pair<std::string, HNDHComponent*>( compID, newComp ) );

    parent->addChildComponent( newComp );

    m_devStatus->debugPrint( 4, m_stringStore, true );

    return HNDH_RESULT_SUCCESS;
}

// Start a new update cycle
void
HNDeviceHealth::startUpdateCycle( time_t updateTimestamp )
{
    // Only allow when enabled
    if( m_enabled == false )
        return;

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
    // Only allow when enabled
    if( m_enabled == false )
        return HNDH_RESULT_SUCCESS;

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
        jsRoot.set( "devStatus", m_devStatus->getPropagatedStatusAsStr() );

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

    // Only allow when enabled
    if( m_enabled == false )
        return;

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

    // Only allow when enabled
    if( m_enabled == false )
        return;

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

    if( comp->getMsgInstancePtr()->clear() == true )
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

    // Only allow when enabled
    if( m_enabled == false )
        return;

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

    if( m_stringStore->fillInstance( fmtCode, vargs, comp->getMsgInstancePtr() ) == HNFS_RESULT_SUCCESS_CHANGED )
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

    // Only allow when enabled
    if( m_enabled == false )
        return;

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

    if( comp->getNoteInstancePtr()->clear() == true )
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

    // Only allow when enabled
    if( m_enabled == false )
        return;

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

    if( m_stringStore->fillInstance( fmtCode, vargs, comp->getNoteInstancePtr() ) == HNFS_RESULT_SUCCESS_CHANGED )
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

    // Only allow when enabled
    if( m_enabled == false )
        return HNDH_RESULT_SUCCESS;

    if( m_devStatus == NULL )
        return changed;

    propagateChild( m_devStatus, changed );

    return changed;
}

void
HNDeviceHealth::populateStrInstJSONObject( void *instObj, HNFSInstance *strInst )
{
    pjs::Object *jsInst = (pjs::Object *) instObj;

    jsInst->set("fmtCode", strInst->getFmtCode() );
}

HNDH_RESULT_T
HNDeviceHealth::addCompJSONObject( void *listPtr, HNDHComponent *comp )
{
    pjs::Array *jsCompList = (pjs::Array *) listPtr;
    pjs::Object jsComp;

    jsComp.set( "id", comp->getID() );
    jsComp.set( "setStatus", comp->getStatusAsStr() );
    jsComp.set( "propagatedStatus", comp->getPropagatedStatusAsStr() );
    jsComp.set( "errCode", comp->getErrorCode() );
    jsComp.set( "updateTime", comp->getLastUpdateTime() );

    pjs::Object jsNameInst;
    populateStrInstJSONObject( &jsNameInst, comp->getNameInstancePtr() );
    jsComp.set( "nameSI", jsNameInst );

    pjs::Object jsDescInst;
    populateStrInstJSONObject( &jsDescInst, comp->getDescInstancePtr() );
    jsComp.set( "descSI", jsDescInst );

    pjs::Object jsMsgInst;
    populateStrInstJSONObject( &jsMsgInst, comp->getMsgInstancePtr() );
    jsComp.set( "msgSI", jsMsgInst );

    pjs::Object jsNoteInst;
    populateStrInstJSONObject( &jsNoteInst, comp->getNoteInstancePtr() );
    jsComp.set( "noteSI", jsNoteInst );

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
        m_devStatus->debugPrint( 4, m_stringStore, true );

        jsRoot.set( "deviceStatus", m_devStatus->getPropagatedStatusAsStr() );
        jsRoot.set( "deviceID", m_deviceID );
        jsRoot.set( "deviceCRC32", m_deviceCRC32 );

        pjs::Object jsCompRoot;

        jsCompRoot.set( "id", m_devStatus->getID() );
        jsCompRoot.set( "setStatus", m_devStatus->getStatusAsStr() );
        jsCompRoot.set( "propagatedStatus", m_devStatus->getPropagatedStatusAsStr() );
        jsCompRoot.set( "errCode", m_devStatus->getErrorCode() );
        jsCompRoot.set( "updateTime", m_devStatus->getLastUpdateTime() );

        pjs::Object jsNameInst;
        populateStrInstJSONObject( &jsNameInst, m_devStatus->getNameInstancePtr() );
        jsCompRoot.set( "nameSI", jsNameInst );

        pjs::Object jsDescInst;
        populateStrInstJSONObject( &jsDescInst, m_devStatus->getDescInstancePtr() );
        jsCompRoot.set( "descSI", jsDescInst );

        pjs::Object jsMsgInst;
        populateStrInstJSONObject( &jsMsgInst, m_devStatus->getMsgInstancePtr() );
        jsCompRoot.set( "msgSI", jsMsgInst );

        pjs::Object jsNoteInst;
        populateStrInstJSONObject( &jsNoteInst, m_devStatus->getNoteInstancePtr() );
        jsCompRoot.set( "noteSI", jsNoteInst );

        pjs::Array jsChildList;
        for( std::vector< HNDHComponent* >::iterator it = m_devStatus->getChildListRef().begin(); it != m_devStatus->getChildListRef().end(); it++ )
        {
            addCompJSONObject( &jsChildList, *it );
        }
        jsCompRoot.set( "children", jsChildList );

        jsRoot.set( "rootComponent", jsCompRoot );
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

HNDHComponent*
HNDeviceHealth::allocateNewComponent( std::string compID )
{
    HNDHComponent *comp = new HNDHComponent( compID );

    std::cout << "allocateNewComponent: " << compID << std::endl;

    return comp;
}

void
HNDeviceHealth::freeComponent( HNDHComponent *comp )
{
    std::cout << "freeComponent: " << comp->getID() << std::endl;
    delete comp;
}

void
HNDeviceHealth::freeComponentTree( HNDHComponent *rootComp )
{
    if( rootComp == NULL )
        return;

    std::vector< HNDHComponent* > &childListRef = rootComp->getChildListRef();
    for( std::vector< HNDHComponent* >::iterator cit = childListRef.begin(); cit != childListRef.end(); cit++ )
    {
        freeComponentTree( *cit );
    }

    freeComponent( rootComp );
}

void
HNDeviceHealth::debugPrint()
{
    std::cout << "==== Health Store - Health Report - start ====" << std::endl;

    if( m_enabled == true )
    {
        std::cout << "enabled: true" << std::endl;
        std::cout << "deviceID: " << m_deviceID << "  deviceCRC32: " << m_deviceCRC32 << "  deviceName: " << m_deviceName << std::endl;

        if( m_devStatus )
            m_devStatus->debugPrint( 0, m_stringStore, true );
    }
    else
    {
        std::cout << "enabled: false" << std::endl;
    }


    std::cout << "==== Health Store - Health Report - end ====" << std::endl;
}

std::string
HNDeviceHealth::renderStringInstance( HNFSInstance *strInst )
{
    std::string rtnStr;

    if( m_stringStore == NULL )
        return rtnStr;

    return m_stringStore->renderInstance( strInst );
}

HNHealthCache::HNHealthCache()
{
    m_strCache = NULL;
}

HNHealthCache::~HNHealthCache()
{

}

void
HNHealthCache::setFormatStringCache( HNFormatStringCache *strCachePtr )
{
    m_strCache = strCachePtr;
}

std::string
HNHealthCache::getHealthReportAsJSON()
{
    return "";
}

void
HNHealthCache::debugPrintHealthReport()
{
    std::cout << "==== Health Cache - Health Report - start ====" << std::endl;

    std::map< std::string, HNDHComponent* >::iterator it;
    for( it = m_devHealthTreeMap.begin(); it != m_devHealthTreeMap.end(); it++ )
    {
        std::cout << "Device: " << it->first << std::endl;

        it->second->debugPrint( 4, m_strCache, true );
    }

    std::cout << "==== Health Cache - Health Report - end ====" << std::endl;
}

std::string
HNHealthCache::renderStringInstance( HNFSInstance *strInst )
{
    std::string rtnStr;

    if( m_strCache == NULL )
        return rtnStr;

    return m_strCache->renderInstance( strInst );    
}

HNDH_RESULT_T
HNHealthCache::updateDeviceHealth( std::string devCRC32ID, std::istream& bodyStream, bool &changed )
{
    // Start off with no change indication
    changed = false;

    std::set< std::string > origCompIDs;

    // {
    //   "deviceCRC32": "535cd1eb",
    //   "deviceID": "hnode2-test-device-default-535cd1eb",
    //   "deviceStatus": "OK",
    //   "enabled": true,
    //   "rootComponent": 
    //   {
    //     "children": [],
    //     "errCode": 0,
    //     "id": "c0",
    //     "msgStr": "",
    //     "name": "Test Name 5",
    //     "noteStr": "",
    //     "propagatedStatus": "OK",
    //     "setStatus": "UNKNOWN",
    //     "updateTime": "Thu Nov 24 13:03:10 2022\n"
    //   }
    // }

    std::cout << "updateDeviceHealth - entry" << std::endl;

    // Parse the json body of the request
    try
    {
        // Attempt to parse the json    
        pjs::Parser parser;
        pdy::Var varRoot = parser.parse( bodyStream );

        // Get a pointer to the root object
        pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

        // Make sure the health service is enabled, if not exit
        if( jsRoot->has( "enabled" ) == false )
        {
            return HNDH_RESULT_FAILURE;
        }
        
        bool enableVal = jsRoot->getValue<bool>( "enabled" );
        if( enableVal != true )
        {   
            return HNDH_RESULT_FAILURE;            
        }

        // Make sure the deviceCRC32 field exists and matches
        if( jsRoot->has( "deviceCRC32" ) == false )
        {
            return HNDH_RESULT_FAILURE;
        }
        
        std::string jsDevCRC32ID = jsRoot->getValue<std::string>( "deviceCRC32" );
        if( jsDevCRC32ID != devCRC32ID )
        {   
            return HNDH_RESULT_FAILURE;            
        }

        // Extract the root component.
        if( jsRoot->has( "rootComponent" ) == false )
        {
            return HNDH_RESULT_FAILURE;
        }

        // Get a pointer to the root object
        pjs::Object::Ptr jsRootComp = jsRoot->getObject( "rootComponent" );

        // Extract root component id and name fields
        if( jsRootComp->has("id") == false )
        {
            return HNDH_RESULT_FAILURE;
        }

        std::string compID = jsRootComp->getValue<std::string>( "id" );

        std::cout << "compID: " << compID << "  devCRC32ID: " << devCRC32ID << std::endl;

        // See if there is already a record for this device in the cache
        std::map< std::string, HNDHComponent* >::iterator dit = m_devHealthTreeMap.find( devCRC32ID );

        if( dit == m_devHealthTreeMap.end() )
        {
            // No existing record for the device, so allocate a new one
            HNDHComponent *newComp = HNDeviceHealth::allocateNewComponent( compID );

            m_devHealthTreeMap.insert( std::pair< std::string, HNDHComponent* >( devCRC32ID, newComp ) );

            dit = m_devHealthTreeMap.find( devCRC32ID );

            changed = true;
        }
        else if( dit->second->getID() != compID )
        {
            // Free the existing component tree
            HNDeviceHealth::freeComponentTree( dit->second );

            m_devHealthTreeMap.erase( dit );

            // Start a new tree with the proper root component
            HNDHComponent *newComp = HNDeviceHealth::allocateNewComponent( compID );

            m_devHealthTreeMap.insert( std::pair< std::string, HNDHComponent* >( devCRC32ID, newComp ) );

            dit = m_devHealthTreeMap.find( devCRC32ID );

            changed = true;
        }

        bool compChange = false;
        handleHealthComponentUpdate( &jsRootComp, dit->second, compChange );

        bool childChange = false;
        handleHealthComponentChildren( &jsRootComp, dit->second, childChange );

        changed = compChange | childChange;
    }
    catch( Poco::Exception ex )
    {
        std::cout << "HNMDARecord::updateHealthInfo exception: " << ex.displayText() << std::endl;
        // Request body was not understood
        return HNDH_RESULT_FAILURE;
    }

    return HNDH_RESULT_SUCCESS;
}

HNDH_RESULT_T
HNHealthCache::handleHealthComponentStrInstanceUpdate( void *jsSIPtr, HNFSInstance *strInstPtr, bool &changed )
{
    // Start off with no change indication
    changed = false;

    std::cout << "handleHealthComponentStrInstanceUpdate - entry" << std::endl;

    // Cast the ptr-ptr back to a POCO JSON Ptr
    pjs::Object::Ptr jsSI = *((pjs::Object::Ptr *) jsSIPtr);

    if( jsSI->has( "fmtCode" ) )
    {
        uint fmtCode = jsSI->getValue<uint>( "fmtCode" );
        std::cout << "      format code: " << fmtCode << std::endl;
        if( strInstPtr->getFmtCode() != fmtCode )
        {
            strInstPtr->setFmtCode( fmtCode );

            // Register fmtCode usage with string store cache, in case its a new one.
            if( m_strCache )
                m_strCache->reportFormatCode( fmtCode );
            
            changed = true;
        }
    }

    return HNDH_RESULT_SUCCESS;
}

HNDH_RESULT_T
HNHealthCache::handleHealthComponentUpdate( void *jsCompPtr, HNDHComponent *compPtr, bool &changed )
{
    // Start off with no change indication
    changed = false;

    std::cout << "handleHealthComponentUpdate - entry" << std::endl;    

    // Cast the ptr-ptr back to a POCO JSON Ptr
    pjs::Object::Ptr jsComp = *((pjs::Object::Ptr *) jsCompPtr);

    if( jsComp->has( "setStatus" ) )
    {
        std::string sStat = jsComp->getValue<std::string>( "setStatus" );
        std::cout << "  comp - status: " << sStat << std::endl;
        if( compPtr->getStatusAsStr() != sStat )
        {
            compPtr->setStatusFromStr( sStat );
            changed = true;
        }
    }

    if( jsComp->has( "propagatedStatus" ) )
    {
        std::string pStat = jsComp->getValue<std::string>( "propagatedStatus" );
        std::cout << "  comp - propagated: " << pStat << std::endl;
        if( compPtr->getPropagatedStatusAsStr() != pStat )
        {
            compPtr->setPropagatedStatusFromStr( pStat );
            changed = true;
        }
    }

    if( jsComp->has( "updateTime" ) )
    {
        time_t uTime = jsComp->getValue<long>( "updateTime" );
        if( compPtr->getLastUpdateTime() != uTime )
        {
            compPtr->setUpdateTimestamp( uTime );
            changed = true;
        }
    }

    if( jsComp->has( "errCode" ) )
    {
        uint eCode = jsComp->getValue<uint>( "errCode" );
        std::cout << "  comp - errCode: " << eCode << std::endl;
        if( compPtr->getErrorCode() != eCode )
        {
            compPtr->setErrorCode( eCode );
            changed = true;
        }
    }

    if( jsComp->isObject( "nameSI" ) )
    {
        bool siChanged = false;
        HNFSInstance *strInstPtr = compPtr->getNameInstancePtr();

        pjs::Object::Ptr jsSI = jsComp->getObject( "nameSI" );
        std::cout << "    parse name string instance" << std::endl;
        handleHealthComponentStrInstanceUpdate( &jsSI, strInstPtr, siChanged );
    }

    if( jsComp->isObject( "descSI" ) )
    {
        bool siChanged = false;
        HNFSInstance *strInstPtr = compPtr->getDescInstancePtr();

        pjs::Object::Ptr jsSI = jsComp->getObject( "descSI" );
        std::cout << "    parse desc string instance" << std::endl;

        handleHealthComponentStrInstanceUpdate( &jsSI, strInstPtr, siChanged );
    }

    if( jsComp->isObject( "noteSI" ) )
    {
        bool siChanged = false;
        HNFSInstance *strInstPtr = compPtr->getNoteInstancePtr();

        pjs::Object::Ptr jsSI = jsComp->getObject( "noteSI" );
        std::cout << "    parse note string instance" << std::endl;

        handleHealthComponentStrInstanceUpdate( &jsSI, strInstPtr, siChanged );
    }

    if( jsComp->isObject( "msgSI" ) )
    {
        bool siChanged = false;
        HNFSInstance *strInstPtr = compPtr->getMsgInstancePtr();

        pjs::Object::Ptr jsSI = jsComp->getObject( "msgSI" );
        std::cout << "    parse msg string instance" << std::endl;

        handleHealthComponentStrInstanceUpdate( &jsSI, strInstPtr, siChanged );
    }

    return HNDH_RESULT_SUCCESS;
}

HNDH_RESULT_T
HNHealthCache::handleHealthComponentChildren( void *jsCompPtr, HNDHComponent *rootComponent, bool &changed )
{
    // Start with no change indication
    changed = false;

    std::cout << "handleHealthComponentChildren - entry" << std::endl;

    // Cast the ptr-ptr back to a POCO JSON Ptr
    pjs::Object::Ptr jsComp = *((pjs::Object::Ptr *) jsCompPtr);

    // Get a list of current component children so that at the end 
    // we can tell if some children need to be deleted.
    std::set< std::string > origChildIDs;
    rootComponent->getChildIDs( origChildIDs );

    pjs::Array::Ptr jsChildArr = jsComp->getArray( "children" );        

    std::cout << "  comp - child array size: " << jsChildArr->size() << std::endl;

    // If the component array doesn't have elements, then exit
    if( jsChildArr->size() == 0 )
    {
        return HNDH_RESULT_SUCCESS;
    }

    // Enumerate through the components in the array
    for( uint i = 0; i < jsChildArr->size(); i++ )
    {
        std::cout << "    child - index: " << i << std::endl;

        // {
        //   "children": [],
        //   "errCode": 0,
        //   "id": "c1",
        //   "msgStr": "",
        //   "name": "test device hc1",
        //   "noteStr": "",
        //   "propagatedStatus": "OK",
        //   "setStatus": "OK",
        //   "updateTime": "Wed Nov 16 12:46:43 2022\n"
        // },
        pjs::Object::Ptr jsComp = jsChildArr->getObject( i );

        std::cout << "===" << std::endl;
        jsComp->stringify(std::cout, 1);
        std::cout << "===" << std::endl;

        // Extract component id and name fields
        if( jsComp->has("id") == false )
            continue;
        std::string compID = jsComp->getValue<std::string>( "id" );

        std::cout << "    child " << i << " - id: " << compID << std::endl;

        // Match with an existing HNDHComponent or create a new one.
        bool compCreated = false;
        HNDHComponent *childComp = rootComponent->getOrCreateChildComponent( compID, compCreated );

        // Indicate that we have visited this component
        origChildIDs.erase( compID );

        // Parse this child's fields
        bool compChange = false;
        handleHealthComponentUpdate( &jsComp, childComp, compChange );

        // Handle any sub components
        bool childChange = false;
        handleHealthComponentChildren( &jsComp, childComp, childChange );

        changed = compCreated | compChange | childChange;

    }

    // Check if any old components need to be deleted at this level
    if( origChildIDs.size() > 0 )
    {
        changed = true;

        for( std::set< std::string >::iterator cit = origChildIDs.begin(); cit != origChildIDs.end(); cit++ )
        {
            std::cout << "Deleting old health component: " << *cit << std::endl;
            rootComponent->deleteChildByID( *cit );
        }
    }

    return HNDH_RESULT_SUCCESS;
}
