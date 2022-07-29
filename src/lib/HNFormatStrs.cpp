#include <cstdarg>
#include <string>
#include <regex>
#include <iostream>
#include <list>

#include <Poco/Checksum.h>

#include "HNFormatStrs.h"

HNFSInstance::HNFSInstance()
{
    m_code = 0;
}

HNFSInstance::HNFSInstance( uint code )
{
    m_code = code;
}

HNFSInstance::~HNFSInstance()
{

}

void 
HNFSInstance::clear()
{
    m_code = 0;
    m_paramList.clear();
    m_resultStr.clear();
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

HNFormatString::HNFormatString( std::string formatStr )
{
    m_formatStr = formatStr;
    calcCode();
}

HNFormatString::~HNFormatString()
{

}

uint 
HNFormatString::getCode()
{
    return m_code;
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
 
    // Put them into the formatSpecs array and temporarily remember thier location the m_formatStr
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

HNFS_RESULT_T 
HNFormatString::applyParameters( va_list vargs, HNFSInstance &instance )
{
    char tmpBuf[4096];
    char pName[64];

    std::vector< std::string > &paramList = instance.getParamListRef();
    std::string &resultStr = instance.getResultStrRef();

    paramList.clear();
    resultStr = m_templateStr;

    for( uint pindx = 0; pindx < m_formatSpecs.size(); pindx++ )
    {
        vsnprintf( tmpBuf, sizeof(tmpBuf), m_formatSpecs[ pindx ].c_str(), vargs );
        paramList.push_back( tmpBuf );

        uint pLen = sprintf( pName, "{%u}", pindx );
        size_t pos = resultStr.find( pName );
        resultStr.replace( pos, pLen, tmpBuf );
    }

    return HNFS_RESULT_SUCCESS;
}

HNFormatStringStore::HNFormatStringStore()
{

}

HNFormatStringStore::~HNFormatStringStore()
{

}

HNFS_RESULT_T
HNFormatStringStore::registerFormatString( std::string formatStr, uint &code )
{
    HNFormatString tmpStr( formatStr );

    code = tmpStr.getCode();

    // Look for an existing entry 
    std::map<uint, HNFormatString>::iterator it = m_formatStrs.find( code );

    // If entry already found then nothing to do return existing code.
    if( it != m_formatStrs.end() )
        return HNFS_RESULT_SUCCESS;

    // Perform additional processing and add the new entry
    HNFS_RESULT_T result = tmpStr.validateFormat(); 
    if( result != HNFS_RESULT_SUCCESS )
    {
        code = 0;
        return result;
    }
  
    m_formatStrs.insert( std::pair<uint, HNFormatString>( code, tmpStr ) );
    
    return HNFS_RESULT_SUCCESS;
}

HNFS_RESULT_T 
HNFormatStringStore::fillInstance( uint fmtCode, va_list vargs, HNFSInstance &instance )
{
    instance.clear();

    // Look up the format string
    std::map< uint, HNFormatString >::iterator it = m_formatStrs.find( fmtCode );

    if( it == m_formatStrs.end() )
    {
        return HNFS_RESULT_FAILURE;
    }

    // Set the instance string code
    instance.setFmtCode( fmtCode );

    // Format the parameters to strings
    it->second.applyParameters( vargs, instance );

    return HNFS_RESULT_SUCCESS;
}

HNFS_RESULT_T 
HNFormatStringStore::fillInstance( uint fmtCode, HNFSInstance &instance, ... )
{
    va_list vargs;
    
    // Look up the format string
    std::map< uint, HNFormatString >::iterator it = m_formatStrs.find( fmtCode );

    if( it == m_formatStrs.end() )
    {
        return HNFS_RESULT_FAILURE;
    }

    // Set the instance string code
    instance.setFmtCode( fmtCode );

    // Format the parameters to strings
    va_start( vargs, instance );    
    it->second.applyParameters( vargs, instance );
    va_end( vargs );

    return HNFS_RESULT_SUCCESS;
}
