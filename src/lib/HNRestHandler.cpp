#include <iostream>

#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"


#include "HNRestHandler.h"

namespace pn = Poco::Net;

HNOperationData::HNOperationData( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatchInf )
: m_dispatchID( dispatchID ), m_opID( operationID ), m_dispInf( dispatchInf )
{
}

HNOperationData::~HNOperationData()
{

}

void 
HNOperationData::setDispatchID( std::string value )
{
    m_dispatchID = value;
}

void 
HNOperationData::setOpID( std::string value )
{
    m_opID = value;
}

std::string 
HNOperationData::getDispatchID()
{
    return m_dispatchID;
}

std::string 
HNOperationData::getOpID()
{
    return m_opID;
}

void
HNOperationData::setReqRsp( void *reqObj, void *rspObj )
{
    m_reqObj = reqObj;
    m_rspObj = rspObj;
}

void 
HNOperationData::setParam( std::string key, std::string value )
{
    m_params.insert( std::pair< std::string, std::string >( key, value ) );
}

void 
HNOperationData::responseSetChunkedTransferEncoding( bool enabled )
{
    ((pn::HTTPServerResponse*) m_rspObj)->setChunkedTransferEncoding( enabled );
}

void 
HNOperationData::responseSetContentType( std::string contentType )
{
    ((pn::HTTPServerResponse*) m_rspObj)->setContentType( contentType );
}

std::ostream& 
HNOperationData::responseSend()
{
    return ((pn::HTTPServerResponse*) m_rspObj)->send();
}

void 
HNOperationData::responseSetStatusAndReason( uint resultCode )
{
    ((pn::HTTPServerResponse*) m_rspObj)->setStatusAndReason( (pn::HTTPResponse::HTTPStatus) resultCode );
}

void
HNOperationData::restDispatch()
{
    m_dispInf->restDispatch( this );
}

HNRestPathElement::HNRestPathElement( HNRPE_TYPE_T typeValue, std::string nameValue )
{
    type = typeValue;
    name = nameValue;
}

HNRestPathElement::~HNRestPathElement()
{

}

HNRPE_TYPE_T 
HNRestPathElement::getType()
{
    return type;
}

std::string 
HNRestPathElement::getName()
{
    return name;
}

HNRestPath::HNRestPath()
{

}

HNRestPath::~HNRestPath()
{

}

void 
HNRestPath::init( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatch )
{
    m_dispatchID = dispatchID;
    m_opID = operationID;
    m_dispatchInf = dispatch;
}

std::string 
HNRestPath::getOpID()
{
   return m_opID;
}

void 
HNRestPath::addPathElement( HNRPE_TYPE_T type, std::string value )
{
    HNRestPathElement npe( type, value );
    m_elements.push_back( npe );
}

HNOperationData* 
HNRestPath::checkForHandler( std::string httpMethod, std::vector< std::string > &urlStrs )
{

    // If number of elements doesn't match
    // then path won't match
    if( m_elements.size() != urlStrs.size() )
    {
        printf( "Path different sizes: %ld %ld\n", m_elements.size(), urlStrs.size() );
        return NULL;
    }

    // If the request type is wrong then move on


    // Start parsing and collecting parameters
    HNOperationData *opData = new HNOperationData( m_dispatchID, m_opID, m_dispatchInf );

    int indx = 0;
    for( std::vector< HNRestPathElement >::iterator it = m_elements.begin(); it != m_elements.end(); it++ )
    {
        switch( it->getType() )
        {
            case HNRPE_TYPE_PATH:
                printf( "Path Check (%d): %s %s\n", indx, it->getName().c_str(), urlStrs[ indx ].c_str() );
                if( it->getName() != urlStrs[ indx ] )
                {
                    delete opData;
                    return NULL;
                }
            break;

            case HNRPE_TYPE_PARAM:
                opData->setParam( it->getName(), urlStrs[ indx ] );
            break;
        }

        indx += 1;
    }

    return opData;
}

#if 0
HNRestHandlerFactory::HNRestHandlerFactory()
{
}

HNRestPath*
HNRestHandlerFactory::addPath( std::string opID, HNRestHandlerFactoryInterface *factory )
{
    HNRestPath newPath;
    HNRestPath *pathPtr;

    // Add new element
    pathList.push_back( newPath );

    // Get pointer
    pathPtr = &pathList.back();
    
    pathPtr->init( opID, factory );

    return pathPtr;
}

pn::HTTPRequestHandler* 
HNRestHandlerFactory::createRequestHandler(const pn::HTTPServerRequest& request)
{
    pn::HTTPRequestHandler *handler = NULL;
    std::vector< std::string > pathStrs;

    //if (request.getURI() == "/")
    //    return new HNodeRestHandler();
    //else

    for( std::vector< HNRestPath >::iterator it = pathList.begin(); it != pathList.end(); it++ )
    {
        handler = it->checkForHandler( request, pathStrs );
        if( handler != NULL )
            return handler;
    }

    return handler;
}
#endif

