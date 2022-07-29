#ifndef __HN_DEVICE_HEALTH_H__
#define __HN_DEVICE_HEALTH_H__

#include <string>
#include <vector>
#include <map>
#include "HNFormatStrs.h"

typedef enum HNDeviceHealthResultEnum {
    HNDH_RESULT_SUCCESS,
    HNDH_RESULT_FAILURE
}HNDH_RESULT_T;

class HNDHFormatStr
{
    public:
        HNDHFormatStr( uint code, std::string formatStr );
       ~HNDHFormatStr();

    private:
        uint        m_code;
        std::string m_formatStr;
};

typedef enum HNDHComponentStandardStatusEnum {
    HNDH_CSTAT_UNKNOWN,
    HNDH_CSTAT_OK,
    HNDH_CSTAT_DEGRADED,
    HNDH_CSTAT_FAILED
}HNDH_CSTAT_T;

class HNDHComponent
{
    public:
        HNDHComponent( std::string compID, std::string componentName );
       ~HNDHComponent(); 

        void clear();

        void setStatus( HNDH_CSTAT_T status );
        void setErrorCode( uint code );

        void setParentID( std::string parentID );

        void clearMsgInstance();
        HNFSInstance& getMsgInstanceRef();

        void clearNoteInstance();
        HNFSInstance& getNoteInstanceRef();

    private:
        std::string m_compID;
        std::string m_compName;

        HNDH_CSTAT_T m_stdStatus;

        uint m_errCode;

        HNFSInstance m_msgInstance;

        HNFSInstance m_noteInstance;

        std::string m_parentID;
};

class HNDeviceHealth
{
    public:
        HNDeviceHealth( HNFormatStringStore *stringStore );
       ~HNDeviceHealth();

        void clear();

        // Register a format string to be used for status reporting, returns a code for that string.
        // HNDH_RESULT_T registerMsgFormat( std::string formatStr, uint &msgCode );

        // Register a component that has monitored health status.
        HNDH_RESULT_T registerComponent( std::string componentName, std::string parentID, std::string &compID );

        void setDeviceStatus( HNDH_CSTAT_T status );
        void setDeviceErrMsg( uint errCode, uint fmtCode, ... );
        void setDeviceNote( uint fmtCode, ... );

        void setComponentStatus( std::string compID, HNDH_CSTAT_T stdStat );
        void setComponentErrMsg( std::string compID, uint errCode, uint fmtCode, ... );
        void setComponentNote( std::string compID, uint fmtCode, ... );

    private:
        // Health status for overall device 
        HNDHComponent m_devStatus;

        // Health status for monitored components
        std::map< std::string, HNDHComponent* > m_compStatus;
        std::vector< HNDHComponent* > m_compOrder;

        // Registered format strings for status and note messages.
        HNFormatStringStore *m_stringStore;
};

#endif // __HN_DEVICE_HEALTH_H__