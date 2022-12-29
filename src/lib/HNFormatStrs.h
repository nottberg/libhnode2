#ifndef __HN_FORMAT_STRS_H__
#define __HN_FORMAT_STRS_H__

#include <cstdio>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <mutex>

class HNFormatString;

typedef enum HNFormatStrsResultEnum {
    HNFS_RESULT_SUCCESS,
    HNFS_RESULT_SUCCESS_CHANGED,
    HNFS_RESULT_FAILURE
}HNFS_RESULT_T;

class HNFSInstance
{
    public:
        HNFSInstance();
        HNFSInstance( uint32_t devCRC32ID, uint code );
       ~HNFSInstance();

        bool clear();
        
        uint32_t getDevCRC32ID();
        uint getFmtCode();

        void setDevCRC32ID( uint32_t devCRC32ID );
        void setFmtCode( uint code );

        std::string getResultStr();
        
    protected:
        std::vector< std::string >& getParamListRef();
        std::string& getResultStrRef();

    private:

        uint32_t m_devCRC32ID;
        uint     m_code;

        std::vector< std::string > m_paramList;

        std::string m_resultStr;

    friend HNFormatString;
};

class HNFormatString
{
    public:
        HNFormatString( uint32_t srcDeviceCRC32ID, std::string formatStr );
       ~HNFormatString();

        uint32_t getDevCRC32ID();
        uint getCode();

        void setDevCRC32ID( uint32_t value );

        HNFS_RESULT_T validateFormat();

        std::string getFormatStr();
        std::string getTemplateStr();

        HNFS_RESULT_T applyParameters( va_list vargs, HNFSInstance *instance );

    private:
        void calcCode();

        uint32_t  m_srcCRC32ID;
        uint      m_code;

        std::string m_formatStr;
        std::string m_templateStr;

        std::vector< std::string > m_formatSpecs;
};

class HNRenderStringIntf
{
    public:
        virtual std::string renderInstance( HNFSInstance *instance ) = 0;
};

class HNFormatStringStore : public HNRenderStringIntf
{
    public:
        HNFormatStringStore();
       ~HNFormatStringStore();

        void setEnabled( uint32_t devCRC32ID );
        bool isEnabled();

        HNFS_RESULT_T registerFormatString( uint32_t srcDevCRC32ID, std::string formatStr, uint &code );

        HNFS_RESULT_T fillInstance( uint fmtCode, va_list vargs, HNFSInstance *instance );
        HNFS_RESULT_T fillInstance( uint fmtCode, HNFSInstance *instance, ... );
        
        virtual std::string renderInstance( HNFSInstance *instance );

        HNFS_RESULT_T getAllFormatStringsJSON( std::ostream& ostr );
        HNFS_RESULT_T getSelectFormatStringsJSON( std::istream& istr, std::ostream& ostr );

        static HNFormatString* allocateFormatString( uint32_t srcDevCRC32ID, std::string formatStr );
        static void freeFormatString( HNFormatString *strPtr );

    private:

        // A mutex for guarding string modifications.
        std::mutex m_updateMutex;

        bool m_enabled;
        uint32_t m_deviceCRC32ID;

        std::map< uint, HNFormatString* > m_formatStrs;
};

// Cache Format Strings for local use
// Used by clients to cache strings from devices
class HNFormatStringCache : public HNRenderStringIntf
{
    public:
        HNFormatStringCache();
       ~HNFormatStringCache();
        
        void reportFormatCode( uint32_t srcDevCRC32ID, uint fmtCode );

        void getUncachedStrRefList( uint32_t srcDevCRC32ID, std::vector< std::string > &strRefList );

        HNFS_RESULT_T updateStringDefinitions( std::string devCRC32ID, std::istream& bodyStream, bool &changed );

        virtual std::string renderInstance( HNFSInstance *instance );

        static HNFormatString* allocateFormatString( uint32_t srcDevCRC32ID, std::string formatStr );
        static void freeFormatString( HNFormatString *strPtr );

    private:

        // A mutex for guarding string modifications.
        std::mutex m_updateMutex;

        // Track any strings that need update from the provider
        std::vector< HNFormatString* > m_needSrcUpdate;

        // Track registered strings
        std::map< uint, HNFormatString* > m_formatStrs;
};

#endif // __HN_FORMAT_STRS_H__