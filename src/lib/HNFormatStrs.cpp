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
    m_devCRC32ID = 0;
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

    if( (m_code != 0) || (m_paramList.size() != 0) )
        changed = true;

    m_code = 0;
    m_paramList.clear();

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

HNFS_RESULT_T
HNFSInstance::setParameters( HNFormatString *formatStr, va_list vargs )
{
    char        tmpBuf[4096];
 
    m_paramList.clear();

    for( uint pindx = 0; pindx < formatStr->getParameterCnt(); pindx++ )
    {
        vsnprintf( tmpBuf, sizeof(tmpBuf), formatStr->getParameterFormatSpec( pindx ).c_str(), vargs );
        m_paramList.push_back( tmpBuf );
    }

    return HNFS_RESULT_SUCCESS_CHANGED;
}

std::string
HNFSInstance::createResolvedString( HNFormatString *formatStr )
{
    std::string builtStr;
    char        pName[64];
 
    builtStr = formatStr->getTemplateStr();

    uint pindx = 0;
    for( std::vector<std::string>::iterator it = m_paramList.begin(); it != m_paramList.end(); it++ )
    {
        uint pLen = sprintf( pName, "{%u}", pindx );
        size_t pos = builtStr.find( pName );
        builtStr.replace( pos, pLen, *it );

        pindx += 1;
    }

    return builtStr;
}

void
HNFSInstance::populateJSONObject( void *jsObj )
{
    pjs::Object *jsInst = (pjs::Object *) jsObj;

    jsInst->set("fmtCode", HNodeID::convertCRC32ToStr( m_code ) );

    pjs::Array pList;

    for( std::vector< std::string >::iterator it = m_paramList.begin(); it != m_paramList.end(); it++ )
    {
        pList.add( *it );
    }

    jsInst->set( "paramList", pList );
}

HNFS_RESULT_T
HNFSInstance::updateFromJSONObject( void *jsSIPtr, bool &changed )
{
    // Cast the ptr-ptr back to a POCO JSON Ptr
    pjs::Object::Ptr jsSI = *((pjs::Object::Ptr *) jsSIPtr);
    
    if( jsSI->has( "fmtCode" ) )
    {
        std::string fmtCodeStr = jsSI->getValue<std::string>( "fmtCode" );
        uint32_t fmtCode = HNodeID::convertStrToCRC32( fmtCodeStr );

        // std::cout << "      format code: " << fmtCode << std::endl;
        if( m_code != fmtCode )
        {
            m_code = fmtCode;
            changed = true;
        }
    }

    if( jsSI->has("paramList") )
    {
        pjs::Array::Ptr jsPArr = jsSI->getArray( "paramList" );        

        std::cout << "=== SI paramList - size: " << jsPArr->size() << " ===" << std::endl;

        if( m_paramList.size() != jsPArr->size() )
        {
            m_paramList.clear();
            changed = true;
        }

        // Enumerate through the components in the array
        for( uint i = 0; i < jsPArr->size(); i++ )
        {
            std::string jsPStr = jsPArr->getElement<std::string>( i );

            std::cout << "    child - index: " << i << "  str: " << jsPStr << std::endl;

            if( m_paramList.size() > i )
            {
                if( m_paramList[i] != jsPStr )
                {
                    m_paramList[i] = jsPStr;
                    changed = true;
                }
            }
            else
            {
                m_paramList.push_back( jsPStr );
                changed = true;
            }
        }
    }

    return HNFS_RESULT_SUCCESS;
}


HNFormatString::HNFormatString( uint32_t srcDevCRC32ID, std::string formatStr )
{
    m_srcCRC32ID = srcDevCRC32ID;
    m_formatStr = formatStr;
    calcCode();
}

HNFormatString::HNFormatString( uint32_t srcDevCRC32ID )
{
    m_srcCRC32ID = srcDevCRC32ID;
}

HNFormatString::~HNFormatString()
{

}

uint32_t
HNFormatString::getDevCRC32ID()
{
    return m_srcCRC32ID;
}

uint32_t 
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
HNFormatString::setCode( uint32_t value )
{
    m_code = value;
}

void
HNFormatString::setFormatStr( std::string value )
{
    m_formatStr = value;
}

void
HNFormatString::setTemplateStr( std::string value )
{
    m_templateStr = value;
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

uint
HNFormatString::getParameterCnt()
{
    return m_formatSpecs.size();
}

std::string
HNFormatString::getParameterFormatSpec( uint index )
{
    return m_formatSpecs[ index ];
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

HNFormatString*
HNFormatStringStore::allocateFormatString( uint32_t srcDevCRC32ID, uint32_t formatCode )
{
    HNFormatString *tmpStr = new HNFormatString( srcDevCRC32ID );
    tmpStr->setCode( formatCode );
    return tmpStr;
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

    // Set the instance parameters
    if( instance->setParameters( it->second, vargs ) == HNFS_RESULT_SUCCESS_CHANGED )
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

    // Format the parameters to strings
    va_start( vargs, instance );
    result = fillInstance( fmtCode, vargs, instance );
    va_end( vargs );

    return result;
}

std::string
HNFormatStringStore::renderInstance( HNFSInstance *instance )
{
    std::ostringstream rtnStr;

    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    uint32_t fmtCode = instance->getFmtCode();

    //std::cout << "HNFormatStringStore::renderInstance - fmtCode: " << HNodeID::convertCRC32ToStr( fmtCode ) << std::endl;

    if( fmtCode == 0 )
        return rtnStr.str();

    std::map< uint32_t, HNFormatString* >::iterator it = m_formatStrs.find( fmtCode );

    if( it == m_formatStrs.end() )
    {
        rtnStr << "Format String not available - fmtCode: " << HNodeID::convertCRC32ToStr( fmtCode );
        return rtnStr.str();
    }

    if( it->second->getTemplateStr().empty() == true )
    {
        rtnStr << "Format String not available - fmtCode: " << HNodeID::convertCRC32ToStr( fmtCode );
        return rtnStr.str();
    }

    //std::cout << "HNFormatStringStore::renderInstance - template: " << it->second->getTemplateStr() << std::endl;

    rtnStr << instance->createResolvedString( it->second );

    return rtnStr.str();    
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

            jsStrObj.set( "fmtCode", HNodeID::convertCRC32ToStr( it->second->getCode() ) );
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
    std::vector< uint32_t > refFmtCodeArray;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_updateMutex );

    // Parse the json body of the request
    try
    {
        // Attempt to parse the json    
        pjs::Parser parser;
        pdy::Var varRoot = parser.parse( istr );

        // Get a pointer to the root object
        pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

        if( jsRoot->has( "strRefs" ) )
        {
            pjs::Array::Ptr jsRefsArr = jsRoot->getArray( "strRefs" );        

            std::cout << "=== HAS strRefs array - size: " << jsRefsArr->size() << " ===" << std::endl;

            // Enumerate through the components in the array
            for( uint i = 0; i < jsRefsArr->size(); i++ )
            {
                std::cout << "    child - index: " << i << std::endl;

                pjs::Object::Ptr jsRef = jsRefsArr->getObject( i );

                // std::cout << "===" << std::endl;
                // jsComp->stringify(std::cout, 1);
                // std::cout << "===" << std::endl;

                // Extract fmtCode field
                if( jsRef->has("fmtCode") == false )
                    continue;
                std::string fmtCodeStr = jsRef->getValue<std::string>( "fmtCode" );
                uint32_t fmtCode = HNodeID::convertStrToCRC32( fmtCodeStr );

                std::cout << "    child " << i << " - fmtCodeStr: " << fmtCodeStr << "  fmtCode: " << fmtCode << std::endl;

                refFmtCodeArray.push_back( fmtCode );
            }
        }
    }
    catch( Poco::Exception ex )
    {
        std::cout << "HNFormatStringStore::getSelectFormatStringsJSON - request parse error: " << ex.displayText() << std::endl;
        return HNFS_RESULT_FAILURE; 
    }


    // Create a json root object
    pjs::Object jsRsp;
    
    jsRsp.set( "enabled", m_enabled );

    if( m_enabled == true )
    {
        pjs::Array jsStrArray;

        jsRsp.set( "deviceCRC32", HNodeID::convertCRC32ToStr( m_deviceCRC32ID ) );

        for( std::vector< uint32_t >::iterator fcit = refFmtCodeArray.begin(); fcit != refFmtCodeArray.end(); fcit++ )
        {
            pjs::Object jsStrObj;

            std::map< uint, HNFormatString* >::iterator it = m_formatStrs.find( *fcit );

            if( it == m_formatStrs.end() )
                continue;

            jsStrObj.set( "fmtCode", HNodeID::convertCRC32ToStr( it->second->getCode() ) );
            jsStrObj.set( "fmtString", it->second->getFormatStr() );
            jsStrObj.set( "templateString", it->second->getTemplateStr() );

            jsStrArray.add( jsStrObj );
        }

        jsRsp.set( "strDefs", jsStrArray );
    }

    // Render the response
    try
    {
        // Write out the generated json
        pjs::Stringifier::stringify( jsRsp, ostr, 1 );
    }
    catch( ... )
    {
        return HNFS_RESULT_FAILURE;
    }

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

    std::cout << "HNFormatStringCache::reportFormatCode - dev: " << devCRC32ID << "  code: " << fmtCode << std::endl;

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

    // Create a new record
    HNFormatString *tmpStr = HNFormatStringStore::allocateFormatString( devCRC32ID, fmtCode );
  
    m_formatStrs.insert( std::pair<uint, HNFormatString*>( tmpStr->getCode(), tmpStr ) );

    // Add it to the set of formats that need to be retrieved
    std::cout << "HNFormatStringCache::reportFormatCode - needs update for code: " << tmpStr->getCode() << std::endl;
    m_needSrcUpdate.push_back( tmpStr );
}

void
HNFormatStringCache::getUncachedStrRefList( uint32_t srcDevCRC32ID, std::vector< std::string > &strRefList )
{
    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    for( std::vector< HNFormatString* >::iterator it = m_needSrcUpdate.begin(); it != m_needSrcUpdate.end(); it++ )
    {
        std::cout << "HNFormatStringCache::getUncachedStrRefList - srcDevCRC32ID: " << HNodeID::convertCRC32ToStr( srcDevCRC32ID ) << "  updCRC32ID: " << HNodeID::convertCRC32ToStr( (*it)->getDevCRC32ID() ) << std::endl;
        if( (*it)->getDevCRC32ID() == srcDevCRC32ID )
        {
            strRefList.push_back( HNodeID::convertCRC32ToStr( (*it)->getCode() ) );
        }
    }

}

HNFS_RESULT_T
HNFormatStringCache::updateStringDefinitions( std::string devCRC32ID, std::istream& bodyStream, bool &changed )
{
    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( m_updateMutex );

    //{
    //    "deviceCRC32" : "535cd1eb",
    //    "enabled" : true,
    //    "strDefs" : [
    //     {
    //        "fmtCode" : 1874885357,
    //        "fmtString" : "Test Name 5",
    //        "templateString" : "Test Name 5"
    //     }]}

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

        // If things are not enabled then exit
        if( jsRoot->has( "enabled") )
        {
            if( jsRoot->getValue<bool>( "enabled" ) == false )
                return HNFS_RESULT_SUCCESS;
        }

        // If the device doesn't match then discard
        if( jsRoot->has( "deviceCRC32") )
        {
            std::string jsDevCRC32Str = jsRoot->getValue< std::string >( "deviceCRC32" );
            std::cout << "updateStringDefinition crc check - devCRC32ID:" << devCRC32ID << "  jsDevCRC32Str: " << jsDevCRC32Str << std::endl;
            if( devCRC32ID != jsDevCRC32Str )
                return HNFS_RESULT_SUCCESS;
        }

        // Look for the string definitions array
        if( jsRoot->has( "strDefs" ) )
        {
            pjs::Array::Ptr jsDefsArr = jsRoot->getArray( "strDefs" );        

            std::cout << "=== HAS strDefs array - size: " << jsDefsArr->size() << " ===" << std::endl;

            // If the component array doesn't have elements, then exit
            if( jsDefsArr->size() == 0 )
            {
                return HNFS_RESULT_SUCCESS;
            }

            // Enumerate through the components in the array
            for( uint i = 0; i < jsDefsArr->size(); i++ )
            {
                std::cout << "    child - index: " << i << std::endl;

                pjs::Object::Ptr jsDef = jsDefsArr->getObject( i );

                // Extract fmtCode field
                if( jsDef->has("fmtCode") == false )
                    continue;
                std::string fmtCodeStr = jsDef->getValue<std::string>( "fmtCode" );
                uint32_t fmtCode = HNodeID::convertStrToCRC32( fmtCodeStr );
                std::cout << "    child " << i << " - fmtCodeStr: " << fmtCodeStr << "  fmtCode: " << fmtCode << std::endl;

                // Lookup the FmtString object
                std::map< uint32_t, HNFormatString* >::iterator it = m_formatStrs.find( fmtCode );

                if( it == m_formatStrs.end() )
                    continue;

                // Update the string definition
                changed = true;

                if( jsDef->has("fmtString") )
                {
                    std::string fmtStr = jsDef->getValue<std::string>( "fmtString" );
                    it->second->setFormatStr( fmtStr );
                }

                if( jsDef->has("templateString") )
                {
                    std::string tmplStr = jsDef->getValue<std::string>( "templateString" );
                    std::cout << "Setting template string: " << tmplStr << std::endl;
                    it->second->setTemplateStr( tmplStr );
                }

                // Remove the object from the needs updating array.
                for( std::vector< HNFormatString* >::iterator it = m_needSrcUpdate.begin(); it != m_needSrcUpdate.end(); it++ )
                {
                    if( (*it)->getCode() == fmtCode )
                    {
                        std::cout << "m_needSrcUpdate - Erasing: " << fmtCodeStr << std::endl;
                        m_needSrcUpdate.erase( it );
                        break;
                    }
                }
            }
        }
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
    std::ostringstream rtnStr;

    // Scope lock
    std::lock_guard<std::mutex> guard( m_updateMutex );

    uint32_t fmtCode = instance->getFmtCode();

    //std::cout << "HNFormatStringCache::renderInstance - fmtCode: " << HNodeID::convertCRC32ToStr( fmtCode ) << std::endl;

    if( fmtCode == 0 )
        return rtnStr.str();

    std::map< uint32_t, HNFormatString* >::iterator it = m_formatStrs.find( fmtCode );

    if( it == m_formatStrs.end() )
    {
        rtnStr << "Format String not available - fmtCode: " << HNodeID::convertCRC32ToStr( fmtCode );
        return rtnStr.str();
    }

    if( it->second->getTemplateStr().empty() == true )
    {
        rtnStr << "Format String not available - fmtCode: " << HNodeID::convertCRC32ToStr( fmtCode );
        return rtnStr.str();
    }

    //std::cout << "HNFormatStringCache::renderInstance - template: " << it->second->getTemplateStr() << std::endl;

    rtnStr << instance->createResolvedString( it->second );

    return rtnStr.str();
}
