#ifndef __HN_FORMAT_STRS_H__
#define __HN_FORMAT_STRS_H__

#include <cstdio>
#include <string>
#include <vector>
#include <map>

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
        HNFSInstance( uint code );
       ~HNFSInstance();

        bool clear();
        
        uint getFmtCode();

        void setFmtCode( uint code );

        std::string getResultStr();
        
    protected:
        std::vector< std::string >& getParamListRef();
        std::string& getResultStrRef();

    private:

        uint m_code;

        std::vector< std::string > m_paramList;

        std::string m_resultStr;

    friend HNFormatString;
};

class HNFormatString
{
    public:
        HNFormatString( std::string formatStr );
       ~HNFormatString();

        uint getCode();

        HNFS_RESULT_T validateFormat();

        HNFS_RESULT_T applyParameters( va_list vargs, HNFSInstance *instance );

    private:
        void calcCode();

        uint        m_code;

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

        HNFS_RESULT_T registerFormatString( std::string formatStr, uint &code );

        HNFS_RESULT_T fillInstance( uint fmtCode, va_list vargs, HNFSInstance *instance );
        HNFS_RESULT_T fillInstance( uint fmtCode, HNFSInstance *instance, ... );
        
        virtual std::string renderInstance( HNFSInstance *instance );

    private:
        std::map< uint, HNFormatString > m_formatStrs;
};

// Cache Format Strings for local use
// Used by clients to cache strings from devices
class HNFormatStringCache : public HNRenderStringIntf
{
    public:
        HNFormatStringCache();
       ~HNFormatStringCache();
        
        void reportFormatCode( uint fmtCode );
        
        virtual std::string renderInstance( HNFSInstance *instance );

    private:
        // std::map< uint, HNFormatString > m_formatStrs;
};

#endif // __HN_FORMAT_STRS_H__