#include <cstdarg>
#include <string>
#include <regex>
#include <iostream>
#include <list>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Checksum.h>

#include "HNodeID.h"
#include "HNFormatStrs.h"

namespace pjs = Poco::JSON;
namespace pdy = Poco::Dynamic;

HNFSInstance::HNFSInstance()
{
    m_code = 0;
}

HNFSInstance::HNFSInstance( uint32_t devCRC32ID, uint code )
{
    m_devCRC32ID = devCRC32ID;
    m_code = code;
}

HNFSInstance::~HNFSInstance()
{

}

bool
HNFSInstance::clear()
{
    bool changed = false;

    if( (m_code != 0) || (m_paramList.size() != 0) || (m_resultStr.empty() == false) )
        changed = true;

    m_code = 0;
    m_paramList.clear();
    m_resultStr.clear();

    return changed;
}

uint32_t
HNFSInstance::getDevCRC32ID()
{
    return m_devCRC32ID;
}

uint
HNFSInstance::getFmtCode()
{
    return m_code;
}

void
HNFSInstance::setDevCRC32ID( uint32_t value )
{
    m_devCRC32ID = value;
}

void
HNFSInstance::setFmtCode( uint code )
{
    m_code = code;
}

std::string
HNFSInstance::getResultStr()
{
    return m_resultStr;
}

std::vector< std::string >&
HNFSInstance::getParamListRef()
{
    return m_paramList;
}

std::string&
HNFSInstance::getResultStrRef()
{
    return m_resultStr;
}

HNFormatString::HNFormatString( uint32_t srcDevCRC32ID, std::string formatStr )
{
    m_srcCRC32ID = srcDevCRC32ID;
    m_formatStr = formatStr;
    calcCode();
}

HNFormatString::~HNFormatString()
{

}

uint32_t
HNFormatString::getDevCRC32ID()
{
    return m_srcCRC32ID;
}

uint 
HNFormatString::getCode()
{
    return m_code;
}

void
HNFormatString::setDevCRC32ID( uint32_t value )
{
    m_srcCRC32ID = value;
}

void 
HNFormatString::calcCode()
{
    // Calculate a crc32 for the format string code
    Poco::Checksum sum;
    sum.update( (const char *) m_formatStr.c_str(), m_formatStr.size() );
    m_code = sum.checksum();
}

HNFS_RESULT_T 
HNFormatString::validateFormat()
{
    m_formatSpecs.clear();
    m_templateStr.clear();

    // Parse out the printf format specifiers
    std::regex formatSpecRE("\\%[0 #+-]?[0-9*]*\\.?[0-9*]*[hl]{0,2}[jztL]?[diuoxXeEfgGaAcpsSn%]");
    auto fs_begin = std::sregex_iterator( m_formatStr.begin(), m_formatStr.end(), formatSpecRE );
    auto fs_end = std::sregex_iterator();
 
    // Put them into the formatSpecs array and temporarily remember thier location in the m_formatStr
    // for later use when building the m_templateStr();
    std::list< std::pair< uint, uint > > locations;
    for( std::sregex_iterator i = fs_begin; i != fs_end; ++i ) 
    {
        std::smatch match = *i;
        std::string mstr = match.str();
        m_formatSpecs.push_back( mstr );

        locations.push_front( std::pair<uint, uint>(match.position(), mstr.size()) );
    }

    // Form the template string by replacing the formatSpecs with "{#}" parameter indications.
    m_templateStr = m_formatStr;
    uint index = m_formatSpecs.size() - 1;
    for( std::list<std::pair<uint,uint>>::iterator it = locations.begin(); it != locations.end(); it++ ) 
    {
        char tmpStr[64];
        snprintf( tmpStr, sizeof(tmpStr), "{%u}", index );
        m_templateStr.replace( it->first, it->second, tmpStr );
        index -= 1;
    }

    return HNFS_RESULT_SUCCESS;
}

std::string
HNFormatString::getFormatStr()
{
    return m_formatStr;
}

std::string
HNFormatString::getTemplateStr()
{
    return m_templateStr;
}

HNFS_RESULT_T 
HNFormatString::applyParameters( va_list vargs, HNFSInstance *instance )
{
    char        tmpBuf[4096];
    std::string builtStr;
    char        pName[64];
    bool        changed = false;
 
    std::vector< std::string > &paramList = instance->getParamListRef();

    paramList.clear();
    builtStr = m_templateStr;

    for( uint pindx = 0; pindx < m_formatSpecs.size(); pindx++ )
    {
        vsnprintf( tmpBuf, sizeof(tmpBuf), m_formatSpecs[ pindx ].c_str(), vargs );
        paramList.push_back( tmpBuf );

        uint pLen = sprintf( pName, "{%u}", pindx );
        size_t pos = builtStr.find( pName );
        builtStr.replace( pos, pLen, tmpBuf );
    }

    std::string &resultStr = instance->getResultStrRef();

    if( resultStr.size() != builtStr.size() )
    {
        resultStr = builtStr;
        return HNFS_RESULT_SUCCESS_CHANGED;
    }
    else if( resultStr != builtStr )
    {
        resultStr = builtStr;
        return HNFS_RESULT_SUCCESS_CHANGED;
    }

    return HNFS_RESULT_SUCCESS;
}

HNFormatStringStore::HNFormatStringStore()
{
    m_enabled = false;
    m_deviceCRC32ID = 0;
}

HNFormatStringStore::~HNFormatStringStore()
{

}

HNFormatString*
HNFormatStringStore::allocateFormatString( uint32_t srcDevCRC32ID, std::string formatStr )
{
    return new HNFormatString( srcDevCRC32ID, formatStr );
}

void
HNFormatStringStore::freeFormatString( HNFormatString *strPtr )
{
    delete strPtr;
}

void 
HNFormatStringStore::setEnabled( uint32_t devCRC32ID )
{
    if( m_enabled == false )
    {
        // Grab the scope lock
        const std::lock_guard< std::mutex > lock( m_updateMutex );

        m_enabled = true;

        m_deviceCRC32ID = devCRC32ID;
    }
}

bool 
HNFormatStringStore::isEnabled()
{
    return m_enabled;
}

HNFS_RESULT_T
HNFormatStringStore::registerFormatString( uint32_t srcDevCRC32ID, std::string formatStr, uint &code )
{
    HNFormatString *tmpStr = HNFormatStringStore::allocateFormatString( srcDevCRC32ID, formatStr );

    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    code = tmpStr->getCode();

    // Look for an existing entry 
    std::map<uint, HNFormatString*>::iterator it = m_formatStrs.find( code );

    // If entry already found then nothing to do return existing code.
    if( it != m_formatStrs.end() )
    {
        HNFormatStringStore::freeFormatString( tmpStr );
        return HNFS_RESULT_SUCCESS;
    }

    // Perform additional processing and add the new entry
    HNFS_RESULT_T result = tmpStr->validateFormat(); 
    if( result != HNFS_RESULT_SUCCESS )
    {
        code = 0;
        HNFormatStringStore::freeFormatString( tmpStr );
        return result;
    }
  
    m_formatStrs.insert( std::pair<uint, HNFormatString*>( code, tmpStr ) );
    
    return HNFS_RESULT_SUCCESS;
}

HNFS_RESULT_T
HNFormatStringStore::fillInstance( uint fmtCode, va_list vargs, HNFSInstance *instance )
{
    bool changed = false;

    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    instance->clear();

    // Look up the format string
    std::map< uint, HNFormatString* >::iterator it = m_formatStrs.find( fmtCode );

    if( it == m_formatStrs.end() )
    {
        return HNFS_RESULT_FAILURE;
    }

    // Set the instance string code
    if( instance->getFmtCode() != fmtCode )
    {
        instance->setFmtCode( fmtCode );
        changed = true;
    }

    // Format the parameters to strings
    if( it->second->applyParameters( vargs, instance ) == HNFS_RESULT_SUCCESS_CHANGED )
    {
        changed = true;
    }

    return (changed == true) ? HNFS_RESULT_SUCCESS_CHANGED : HNFS_RESULT_SUCCESS;
}

HNFS_RESULT_T
HNFormatStringStore::fillInstance( uint fmtCode, HNFSInstance *instance, ... )
{
    va_list vargs;
    HNFS_RESULT_T result = HNFS_RESULT_FAILURE;

    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    // Format the parameters to strings
    va_start( vargs, instance );
    result = fillInstance( fmtCode, vargs, instance );
    va_end( vargs );

    return result;
}

std::string
HNFormatStringStore::renderInstance( HNFSInstance *instance )
{
    std::string rtnStr;

    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    rtnStr = "FIXME HNFormatStringStore test return string";

    return rtnStr;
}

HNFS_RESULT_T
HNFormatStringStore::getAllFormatStringsJSON( std::ostream& ostr )
{
    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_updateMutex );

    // Create a json root object
    pjs::Object jsRoot;
    
    jsRoot.set( "enabled", m_enabled );

    if( m_enabled == true )
    {
        jsRoot.set( "deviceCRC32", HNodeID::convertCRC32ToStr( m_deviceCRC32ID ) );

        pjs::Array jsStrArray;

        for( std::map< uint, HNFormatString* >::iterator it = m_formatStrs.begin(); it != m_formatStrs.end(); it++ )
        {
            pjs::Object jsStrObj;

            jsStrObj.set( "fmtCode", it->second->getCode() );
            jsStrObj.set( "fmtString", it->second->getFormatStr() );
            jsStrObj.set( "templateString", it->second->getTemplateStr() );

            jsStrArray.add( jsStrObj );
        }

        jsRoot.set( "strDefs", jsStrArray );
    }

    // Render the response
    try
    {
        // Write out the generated json
        pjs::Stringifier::stringify( jsRoot, ostr, 1 );
    }
    catch( ... )
    {
        return HNFS_RESULT_FAILURE;
    }
    
    return HNFS_RESULT_SUCCESS;
}

HNFS_RESULT_T
HNFormatStringStore::getSelectFormatStringsJSON( std::istream& istr, std::ostream& ostr )
{
    return HNFS_RESULT_SUCCESS;
}

HNFormatStringCache::HNFormatStringCache()
{

}

HNFormatStringCache::~HNFormatStringCache()
{

}

HNFormatString*
HNFormatStringCache::allocateFormatString( uint32_t srcDevCRC32ID, std::string formatStr )
{
    return new HNFormatString( srcDevCRC32ID, formatStr );
}

void
HNFormatStringCache::freeFormatString( HNFormatString *strPtr )
{
    delete strPtr;
}

void
HNFormatStringCache::reportFormatCode( uint32_t devCRC32ID, uint fmtCode )
{
    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    std::cout << "HNFormatStringCache::reportFormatCode - code: " << fmtCode << std::endl;

    // Empty string is not valid
    if( fmtCode == 0 )
        return;

    // See if we already have the string locally
    std::map< uint, HNFormatString* >::iterator it = m_formatStrs.find( fmtCode );

    if( it != m_formatStrs.end() )
    {
        // String already has local record
        return;
    }

    // Add it to the set of formats that need to be retrieved
    m_needSrcUpdate.push_back( it->second );
}

void
HNFormatStringCache::getUncachedStrRefList( uint32_t srcDevCRC32ID, std::vector< std::string > &strRefList )
{
    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    for( std::vector< HNFormatString* >::iterator it = m_needSrcUpdate.begin(); it != m_needSrcUpdate.end(); it++ )
    {
        if( (*it)->getDevCRC32ID() == srcDevCRC32ID )
        {
            strRefList.push_back("fixme");
        }
    }

}

HNFS_RESULT_T
HNFormatStringCache::updateStringDefinitions( std::string devCRC32ID, std::istream& bodyStream, bool &changed )
{
    // {
    // }

    // Parse the json body of the request
    try
    {
        // Attempt to parse the json    
        pjs::Parser parser;
        pdy::Var varRoot = parser.parse( bodyStream );

        // Get a pointer to the root object
        pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

        std::cout << "updateStringDefinition json:" << std::endl;
        jsRoot->stringify( std::cout, 1 );

#if 0
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

        // std::cout << "compID: " << compID << "  devCRC32ID: " << devCRC32ID << std::endl;

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

            m_devHealthTreeMap.clear();

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
#endif
    }
    catch( Poco::Exception ex )
    {
        std::cout << "HNFormatStringCache::updateStringDefinitions exception: " << ex.displayText() << std::endl;
        // Request body was not understood
        return HNFS_RESULT_FAILURE;
    }

    return HNFS_RESULT_SUCCESS;
}

std::string
HNFormatStringCache::renderInstance( HNFSInstance *instance )
{
    std::string rtnStr;

    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    rtnStr = "FIXME HNFormatStringCache test return string";

    return rtnStr;
}
