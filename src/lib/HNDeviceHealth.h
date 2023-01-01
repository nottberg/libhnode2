#ifndef __HN_DEVICE_HEALTH_H__
#define __HN_DEVICE_HEALTH_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>

#include "HNHttpEventClient.h"
#include "HNFormatStrs.h"

#define HNDH_ROOT_COMPID  "c0"

typedef enum HNDeviceHealthResultEnum {
    HNDH_RESULT_SUCCESS,
    HNDH_RESULT_FAILURE
}HNDH_RESULT_T;

#if 0
class HNDHFormatStr
{
    public:
        HNDHFormatStr( uint code, std::string formatStr );
       ~HNDHFormatStr();

    private:
        uint        m_code;
        std::string m_formatStr;
};
#endif

typedef enum HNDHComponentStandardStatusEnum {
    HNDH_CSTAT_UNKNOWN,
    HNDH_CSTAT_OK,
    HNDH_CSTAT_DEGRADED,
    HNDH_CSTAT_FAILED
}HNDH_CSTAT_T;

class HNDHComponent
{
    public:
        HNDHComponent( uint32_t devCRC32ID, std::string compID );
       ~HNDHComponent(); 

        void clear();
       
        void setID( std::string idStr );

        void setStatus( HNDH_CSTAT_T status );
        void setStatusFromStr( std::string status );

        void setPropagatedStatus( HNDH_CSTAT_T status );
        void setPropagatedStatusFromStr( std::string status );

        void setErrorCode( uint code );

        void setParentID( std::string parentID );

        void setUpdateTimestamp( time_t updateTS );
        //void setUpdateTimestampFromStr( std::string updateTS );

        std::string getID();

        HNDH_CSTAT_T getStatus();
        std::string getStatusAsStr();

        HNDH_CSTAT_T getPropagatedStatus();
        std::string getPropagatedStatusAsStr();

        time_t getLastUpdateTime();
        std::string getLastUpdateTimeAsStr();

        uint getErrorCode();
        
        /*
        std::string getRenderedName();
        std::string getRenderedDesc();
        std::string getRenderedNote();

        std::string getRenderedMsg();
        */

        void clearNameInstance();
        HNFSInstance* getNameInstancePtr();

        void clearDescInstance();
        HNFSInstance* getDescInstancePtr();

        void clearNoteInstance();
        HNFSInstance* getNoteInstancePtr();

        void clearMsgInstance();
        HNFSInstance* getMsgInstancePtr();

        HNDHComponent *getChildComponent( std::string compID );
        HNDHComponent *getChildComponent( uint index );

        HNDHComponent *getOrCreateChildComponent( std::string compID, bool &childCreated );

        void addChildComponent( HNDHComponent *childComp );

        void getChildIDs( std::set< std::string > &childIDs );
        void deleteChildByID( std::string compID );

        std::vector< HNDHComponent* >& getChildListRef();

        void debugPrint( uint offset, HNRenderStringIntf *renderIntf, bool printChildren );

    private:
        uint32_t    m_devCRC32ID;

        std::string m_compID;

        HNDH_CSTAT_T m_stdStatus;

        HNDH_CSTAT_T m_propagatedStatus;

        HNFSInstance m_nameInstance;
        HNFSInstance m_descInstance;
        HNFSInstance m_noteInstance;

        uint m_errCode;
        HNFSInstance m_msgInstance;

        std::string m_parentID;

        time_t m_lastUpdateTS;
        
        std::vector< HNDHComponent* > m_children;
};

class HNDeviceHealth
{
    public:
        HNDeviceHealth( HNFormatStringStore *stringStore, HNHttpEventClient *evClient );
       ~HNDeviceHealth();

        void setEnabled( uint32_t devCRC32ID );
        bool isEnabled();

        void clear();

        // Initialize the root component
        HNDH_RESULT_T updateDeviceInfo( std::string deviceID, std::string deviceName );

        // Register a component that has monitored health status.
        HNDH_RESULT_T registerComponent( std::string componentName, std::string parentID, std::string &compID );

        // Start a new update cycle
        void startUpdateCycle( time_t updateTimestamp );

        // Complete an update cycle, return whether values changed.
        bool completeUpdateCycle();

        void setComponentStatus( std::string compID, HNDH_CSTAT_T stdStatus );

        void clearComponentDesc( std::string compID );
        void setComponentDesc( std::string compID, std::string descStr );

        void clearComponentErrMsg( std::string compID );
        void setComponentErrMsg( std::string compID, uint errCode, uint fmtCode, ... );

        void clearComponentNote( std::string compID );
        void setComponentNote( std::string compID, uint fmtCode, ... );

        HNDH_RESULT_T getRestJSON( std::string &jsonStr );
        HNDH_RESULT_T getRestJSON( std::ostream &oStream );

        void clearSinkMapping();
        void checkSinkMapping( std::string uri );
        std::string getSinkMapping();

        static HNDHComponent* allocateNewComponent( uint32_t devCRC32ID, std::string compID );
        static void freeComponent( HNDHComponent *rootComp );
        static void freeComponentTree( HNDHComponent *rootComp );

        void debugPrint();

    private:
        HNDH_RESULT_T allocUniqueID( std::string &compID );

        HNDH_CSTAT_T propagateChild( HNDHComponent *comp, bool &changed );
        bool propagateStatus();

        //void populateStrInstJSONObject( void *instObj, HNFSInstance *strInst );
        HNDH_RESULT_T addCompJSONObject( void *listPtr, HNDHComponent *comp );

        std::string renderStringInstance( HNFSInstance *strInst );

        // Guard for multi-threaded access to health data.
        std::mutex m_accessMutex;

        // Is device health reporting enabled.
        bool m_enabled;

        // The device ID
        std::string m_deviceID;
        uint32_t m_deviceCRC32ID;
        std::string m_deviceName;

        // Root of health status tree 
        HNDHComponent *m_devStatus;

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

        // Handle sends to a mapped health sink
        HNHttpEventClient *m_evClient;

        // Keep track of the sink uri for push notifications of health changes.
        std::string m_sinkURI;
};

// Used by clients to cache health information from multiple 
// device providers
class HNHealthCache
{
    public:
        HNHealthCache();
       ~HNHealthCache();

        void setFormatStringCache( HNFormatStringCache *strCachePtr );

        HNDH_RESULT_T updateDeviceHealth( uint32_t devCRC32ID, std::istream& bodyStream, bool &changed );

        std::string getHealthReportAsJSON();

        void debugPrintHealthReport();

    private:

        std::string renderStringInstance( HNFSInstance *strInst );

        HNFormatStringCache *m_strCache;

        std::map< uint32_t, HNDHComponent* > m_devHealthTreeMap;

        HNDH_RESULT_T handleHealthComponentStrInstanceUpdate( void *jsSIPtr, HNFSInstance *strInstPtr, bool &changed );
        HNDH_RESULT_T handleHealthComponentUpdate( void *jsCompPtr, HNDHComponent *compPtr, bool &changed );
        HNDH_RESULT_T handleHealthComponentChildren( void *jsCompPtr, HNDHComponent *rootComponent, bool &changed );

};

#endif // __HN_DEVICE_HEALTH_H__