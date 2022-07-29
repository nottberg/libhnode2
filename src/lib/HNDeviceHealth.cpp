#include <cstdarg>
#include <cstdio>

#include <Poco/Checksum.h>

#include "HNDeviceHealth.h"

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
    m_errCode = 0;
    m_parentID.clear();
    m_msgInstance.clear();
    m_noteInstance.clear();
}

void 
HNDHComponent::setStatus( HNDH_CSTAT_T status )
{
    m_stdStatus = status;
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

HNDeviceHealth::HNDeviceHealth( HNFormatStringStore *stringStore )
: m_devStatus( "Device Overall Health", "c0")
{
    m_stringStore = stringStore;
}

HNDeviceHealth::~HNDeviceHealth()
{

}

void 
HNDeviceHealth::clear()
{
    m_devStatus.clear();
    m_compStatus.clear();
}

/*
HNDH_RESULT_T 
HNDeviceHealth::registerMsgFormat( std::string formatStr, uint &msgCode )
{
    if( m_stringStore == NULL )
        return HNDH_RESULT_FAILURE;

    if( m_stringStore->registerFormatString( formatStr, msgCode ) != HNFS_RESULT_SUCCESS )
        return HNDH_RESULT_FAILURE;

    return HNDH_RESULT_SUCCESS;
}
*/

HNDH_RESULT_T 
HNDeviceHealth::registerComponent( std::string componentName, std::string parentID, std::string &compID )
{
    // If there is a parentID then ensure it already exists in the list
    if( parentID.empty() == false )
    {
        // Look for an existing entry 
        std::map<std::string, HNDHComponent* >::iterator it = m_compStatus.find( parentID );

        // If couldn't find referenced parent then fail.
        if( it == m_compStatus.end() )
            return HNDH_RESULT_FAILURE;
    }

    // Allocate a new component ID and add this component.
    compID = "c1";

    HNDHComponent *newComp = new HNDHComponent( compID, componentName );

    if( parentID.empty() == false )
        newComp->setParentID( parentID );

    m_compStatus.insert( std::pair<std::string, HNDHComponent*>( compID, newComp ) );
    m_compOrder.push_back( newComp );

    return HNDH_RESULT_SUCCESS;
}

void 
HNDeviceHealth::setDeviceStatus( HNDH_CSTAT_T status )
{
    m_devStatus.setStatus( status );
}

void 
HNDeviceHealth::setDeviceErrMsg( uint errCode, uint fmtCode, ... )
{
    char errBuf[512];
    va_list vargs;

    if( m_stringStore == NULL )
        return;

    va_start( vargs, fmtCode );

    m_devStatus.setErrorCode( errCode );

    m_stringStore->fillInstance( fmtCode, vargs, m_devStatus.getMsgInstanceRef() );

    va_end( vargs );

/*
    std::map<uint, HNDHFormatStr>::iterator it = m_formatStrs.find( fmtCode );

    if( it == m_formatStrs.end() )
        return;

    va_start( args, it->second.getFormatStr().c_str() );
    vsnprintf( errBuf, sizeof(errBuf), it->second.getFormatStr().c_str(), args );
    va_end(args);
    
    m_devStatus.setErrMsgCode( fmtCode );
    m_devStatus.setErrStr( errBuf );
*/

}

void 
HNDeviceHealth::setDeviceNote( uint fmtCode, ... )
{

}

void 
HNDeviceHealth::setComponentStatus( std::string compID, HNDH_CSTAT_T stdStat )
{

}

void 
HNDeviceHealth::setComponentErrMsg( std::string compID, uint errCode, uint fmtCode, ... )
{

}

void 
HNDeviceHealth::setComponentNote( std::string compID, uint fmtCode, ... )
{

}
