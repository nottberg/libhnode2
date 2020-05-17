#ifndef _HN_REST_HANDLER_H_
#define _HN_REST_HANDLER_H_

#include <string>
#include <map>

#define HNR_HTTP_OK                      200
#define HNR_HTTP_BAD_REQUEST             400
#define HNR_HTTP_CONFLICT                409
#define HNR_HTTP_NOT_FOUND               404
#define HNR_HTTP_INTERNAL_SERVER_ERROR   500
#define HNR_HTTP_NOT_IMPLEMENTED         501

class HNOperationData;

class HNRestDispatchInterface
{
    private:

    public:
        virtual void restDispatch( HNOperationData *opData ) = 0;
};

class HNOperationData
{
    private:
        std::string m_dispatchID;
        std::string m_opID;
     
        HNRestDispatchInterface *m_dispInf;

        std::map< std::string, std::string > m_params;

        void *m_reqObj;
        void *m_rspObj;

    public:
        HNOperationData( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatchInf  );
       ~HNOperationData();

        void setDispatchID( std::string value );
        void setOpID( std::string value );

        void setReqRsp( void *request, void *response );

        std::string getDispatchID();
        std::string getOpID();

        void setParam( std::string key, std::string value );

        bool getParam( std::string key, std::string &value );

        std::istream& requestBody();

        void responseSetChunkedTransferEncoding( bool enabled );
        void responseSetContentType( std::string contentType );
        std::ostream& responseSend();
        void responseSetStatusAndReason( uint resultCode );

        void restDispatch();
};

typedef enum HNRestPathElementTypeEnum
{
    HNRPE_TYPE_PATH,
    HNRPE_TYPE_PARAM
}HNRPE_TYPE_T;

class HNRestPathElement
{
    private:
        HNRPE_TYPE_T type;
        std::string  name;

    public:
        HNRestPathElement( HNRPE_TYPE_T typeValue, std::string nameValue );
       ~HNRestPathElement();

        HNRPE_TYPE_T getType();
        std::string getName();
};

class HNRestPath
{
    private:
        std::string m_method;
        std::vector< HNRestPathElement > m_elements;

        std::string m_dispatchID;
        std::string m_opID;
        HNRestDispatchInterface *m_dispatchInf;

    public:
        HNRestPath();
       ~HNRestPath();

        void init( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatch );

        void setMethod( std::string value );

        std::string getOpID();

        void addPathElement( HNRPE_TYPE_T type, std::string value );

        HNOperationData* checkForHandler( std::string httpMethod, std::vector< std::string > &urlStrs );
};

#endif // _H_HN_REST_HANDLER_H_
