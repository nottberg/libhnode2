#include <Poco/Checksum.h>

#include "HNDeviceHealth.h"

HNDHFormatStr::HNDHFormatStr( uint code, std::string formatStr )
{
    m_code = code;
    m_formatStr = formatStr;
}

HNDHFormatStr::~HNDHFormatStr()
{

}

HNDHComponent::HNDHComponent( std::string compID, std::string componentName )
{
    m_compID = compID;
    m_compName = componentName;

    m_stdStatus = HNDH_CSTAT_UNKNOWN;

    m_errCode  = 0;
    m_msgCode  = 0;
    m_noteCode = 0;
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
    m_msgCode = 0;
    m_msgParams.clear();
    m_noteCode = 0;
    m_noteParams.clear();
}

HNDeviceHealth::HNDeviceHealth()
: m_devStatus( "Device Overall Health", "c0")
{

}

HNDeviceHealth::~HNDeviceHealth()
{

}

void 
HNDeviceHealth::clear()
{
    m_devStatus.clear();
    m_compStatus.clear();
    m_formatStrs.clear();
}

HNDH_RESULT_T 
HNDeviceHealth::registerMsgFormat( std::string formatStr, uint &msgCode )
{
    // Calculate a crc32 for the format string code
    Poco::Checksum sum;
    sum.update( (const char *) formatStr.c_str(), formatStr.size() );
    msgCode = sum.checksum();

    // Look for an existing entry 
    std::map<uint, HNDHFormatStr>::iterator it = m_formatStrs.find( msgCode );

    // If no entry found then add this string
    if( it == m_formatStrs.end() )
    {
        HNDHFormatStr newFS( msgCode, formatStr );

        m_formatStrs.insert( std::pair<uint, HNDHFormatStr>(msgCode, newFS) );
    }

    return HNDH_RESULT_SUCCESS;
}

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

    m_compStatus.insert( std::pair<std::string, HNDHComponent*>( compID, newComp ) );
    m_compOrder.push_back( newComp );

    return HNDH_RESULT_SUCCESS;
}

void 
HNDeviceHealth::setDeviceStatus( HNDH_CSTAT_T status )
{

}

void 
HNDeviceHealth::setDeviceErrMsg( std::string compID, uint errCode, uint fmtCode, ... )
{

}

void 
HNDeviceHealth::setDeviceNote( std::string compID, uint fmtCode, ... )
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
