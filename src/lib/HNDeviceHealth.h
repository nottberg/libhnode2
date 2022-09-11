#ifndef __HN_DEVICE_HEALTH_H__
#define __HN_DEVICE_HEALTH_H__

#include <string>
#include <vector>
#include <map>
#include <mutex>

#include "HNFormatStrs.h"

#define HNDH_ROOT_COMPID  "c0"

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
       
        void setID( std::string idStr );

        void setName( std::string compName );

        void setStatus( HNDH_CSTAT_T status );

        void setPropagatedStatus( HNDH_CSTAT_T status );

        void setErrorCode( uint code );

        void setParentID( std::string parentID );

        void setUpdateTimestamp( time_t updateTS );

        std::string getID();
        std::string getComponentName();

        HNDH_CSTAT_T getStatus();
        std::string getStatusAsStr();

        HNDH_CSTAT_T getPropagatedStatus();
        std::string getPropagatedStatusAsStr();

        time_t getLastUpdateTime();
        std::string getLastUpdateTimeAsStr();

        uint getErrorCode();
        
        std::string getRenderedMsg();
        std::string getRenderedNote();

        void clearMsgInstance();
        HNFSInstance& getMsgInstanceRef();

        void clearNoteInstance();
        HNFSInstance& getNoteInstanceRef();

        void addChildComponent( HNDHComponent *childComp );

        std::vector< HNDHComponent* >& getChildListRef();

    private:
        std::string m_compID;
        std::string m_compName;

        HNDH_CSTAT_T m_stdStatus;

        HNDH_CSTAT_T m_propagatedStatus;

        uint m_errCode;

        HNFSInstance m_msgInstance;

        HNFSInstance m_noteInstance;

        std::string m_parentID;

        time_t m_lastUpdateTS;
        
        std::vector< HNDHComponent* > m_children;
};

class HNDeviceHealth
{
    public:
        HNDeviceHealth( HNFormatStringStore *stringStore );
       ~HNDeviceHealth();

        void setEnabled();
        bool isEnabled();

        void clear();

        // Initialize the root component
        HNDH_RESULT_T init( std::string deviceID, std::string deviceCRC32, std::string deviceName );

        // Register a component that has monitored health status.
        HNDH_RESULT_T registerComponent( std::string componentName, std::string parentID, std::string &compID );

        // Start a new update cycle
        void startUpdateCycle( time_t updateTimestamp );

        // Complete an update cycle, return whether values changed.
        bool completeUpdateCycle();

        void setComponentStatus( std::string compID, HNDH_CSTAT_T stdStatus );

        void clearComponentErrMsg( std::string compID );
        void setComponentErrMsg( std::string compID, uint errCode, uint fmtCode, ... );

        void clearComponentNote( std::string compID );
        void setComponentNote( std::string compID, uint fmtCode, ... );

        HNDH_RESULT_T getRestJSON( std::string &jsonStr );
        HNDH_RESULT_T getRestJSON( std::ostream &oStream );

        void clearService();
        void setServiceRootURIFromStr( std::string value );
        std::string getServiceRootURIAsStr();

    private:
        HNDH_RESULT_T allocUniqueID( std::string &compID );

        HNDH_CSTAT_T propagateChild( HNDHComponent *comp, bool &changed );
        bool propagateStatus();

        HNDH_RESULT_T addCompJSONObject( void *listPtr, HNDHComponent *comp );

        // Guard for multi-threaded access to health data.
        std::mutex m_accessMutex;

        // Is device health reporting enabled.
        bool m_enabled;

        // The device ID
        std::string m_deviceID;
        std::string m_deviceCRC32;

        // Root of health status tree 
        HNDHComponent m_devStatus;

        // Health status for monitored components
        std::map< std::string, HNDHComponent* > m_compStatus;

        // Make sure start-update, complete-update calls wrap set status calls.
        bool m_lockedForUpdate;

        // Timestamp for current update cycle
        time_t m_updateCycleTimestamp;

        // Tracking variable for status changes
        bool m_statusChange;

        // Access to registered format strings for status and note messages.
        HNFormatStringStore *m_stringStore;

        // Keep track of the service url for push notifications of health changes.
        std::string m_serviceRootURI;
};

#endif // __HN_DEVICE_HEALTH_H__