#include <iostream>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/URI.h"
#include "Poco/String.h"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "HNHttpServer.h"

namespace pjs = Poco::JSON;
namespace pdy = Poco::Dynamic;
namespace pn = Poco::Net;

// Define these locally so it is not in the header file.
class HNRestHandler: public pn::HTTPRequestHandler
{
    private:
        HNOperationData *m_opData;

    public:
        HNRestHandler( HNOperationData *operationData );
       ~HNRestHandler();

        void handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response );
};

HNRestHandler::HNRestHandler( HNOperationData *operationData )
:m_opData( operationData )
{

}

HNRestHandler::~HNRestHandler()
{
    std::cout << "HNRestHandler - deconstruct" << std::endl;
    delete m_opData;
}

void
HNRestHandler::handleRequest( pn::HTTPServerRequest& request, pn::HTTPServerResponse& response )
{
    m_opData->setReqRsp( &request, &response );
    m_opData->restDispatch();
}

class HNRestHandlerFactory: public pn::HTTPRequestHandlerFactory
{
    public:
        HNRestHandlerFactory();

        HNRestPath *addPath( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatchInf );

        pn::HTTPRequestHandler* createRequestHandler( const pn::HTTPServerRequest& request );

    private:
        std::vector< HNRestPath > pathList;
};

HNRestHandlerFactory::HNRestHandlerFactory()
{

}

HNRestPath *
HNRestHandlerFactory::addPath( std::string dispatchID, std::string operationID, HNRestDispatchInterface *dispatchInf )
{
    HNRestPath newPath;
    HNRestPath *pathPtr;

    // Add new element
    pathList.push_back( newPath );

    // Get pointer
    pathPtr = &pathList.back();
    
    pathPtr->init( dispatchID, operationID, dispatchInf );

    return pathPtr;
}

pn::HTTPRequestHandler* 
HNRestHandlerFactory::createRequestHandler( const pn::HTTPServerRequest& request )
{
    HNOperationData *opData = NULL;
    std::string method;
    std::vector< std::string > pathStrs;

    Poco::URI uri( request.getURI() );    

    //if (request.getURI() == "/")
    //    return new HNodeRestHandler();
    //else

    std::cout << "Server Request method: " << request.getMethod() << std::endl;
    std::cout << "Server Request URI: " << uri.toString() << std::endl;

    uri.getPathSegments( pathStrs );

    for( std::vector< HNRestPath >::iterator it = pathList.begin(); it != pathList.end(); it++ )
    {
        std::cout << "Check handler: " << it->getOpID() << std::endl;
        opData = it->checkForHandler( method, pathStrs );
        if( opData != NULL )
            return new HNRestHandler( opData );
    }

    // Should return default error handler instead?
    return NULL;
}


HNHttpServer::HNHttpServer()
{
    m_port   = 8080;
    m_srvPtr = NULL;

    m_facPtr = new HNRestHandlerFactory();
}

HNHttpServer::~HNHttpServer()
{
    if( m_srvPtr )
        delete ((Poco::Net::HTTPServer*)m_srvPtr);
}

void
HNHttpServer::registerEndpointsFromOpenAPI( std::string dispatchID, HNRestDispatchInterface *dispatchInf, std::string openAPIJson )
{
    HNRestPath *path;

    // Invoke the json parser
    try
    {
        // Attempt to parse the json    
        pjs::Parser parser;
        pdy::Var varRoot = parser.parse( openAPIJson );

        // Get a pointer to the root object
        pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

        if( jsRoot->isObject( "paths" ) == false )
            return;

        // Extract the paths object
        pjs::Object::Ptr jsPathList = jsRoot->getObject( "paths" );

        // Iterate through the fields, each one represents a path
        for( pjs::Object::ConstIterator pit = jsPathList->begin(); pit != jsPathList->end(); pit++ )
        {
            Poco::URI uri( pit->first );
            std::vector< std::string > pathStrs;

            uri.getPathSegments( pathStrs );

            std::cout << "regend - uri: " << uri.toString() << std::endl;
            std::cout << "regend - segcnt: " << pathStrs.size() << std::endl;

            pjs::Object::Ptr jsPath = pit->second.extract< pjs::Object::Ptr >();

            // Iterate through the fields, each one represents a supported HTTP Method
            for( pjs::Object::ConstIterator mit = jsPath->begin(); mit != jsPath->end(); mit++ )
            {
                std::cout << "regend - method: " << mit->first << std::endl;

                pjs::Object::Ptr jsMethod = mit->second.extract< pjs::Object::Ptr >();

                std::string opID = jsMethod->getValue< std::string >( "operationId" );
                std::cout << "regend - opID: " << opID << std::endl;

                path = ((HNRestHandlerFactory *) m_facPtr)->addPath( dispatchID, opID,  dispatchInf );
             
                for( std::vector< std::string >::iterator sit = pathStrs.begin(); sit != pathStrs.end(); sit++ )
                {
                    path->addPathElement( HNRPE_TYPE_PATH, *sit );
                } 
            }


#if 0
                HNCSection *secPtr;
                config.updateSection( it->first, &secPtr );

                pjs::Object::Ptr jsSec = it->second.extract< pjs::Object::Ptr >();

                for( pjs::Object::ConstIterator sit = jsSec->begin(); sit != jsSec->end(); sit++ )
                {

                    if( sit->second.isString() == true )
                    {    
                        secPtr->updateValue( sit->first, sit->second.extract< std::string >() ); 
                    }
                    else if( jsSec->isArray( sit ) == true )
                    {
                        HNCObjList *listPtr;
                        secPtr->updateList( sit->first, &listPtr );

                        pjs::Array::Ptr jsArr = jsSec->getArray( sit->first );
                  
                        uint index = 0;
                        for( uint index = 0; index < jsArr->size(); index++ )
                        {
                            if( jsArr->isObject( index ) == true )
                            {
                                pjs::Object::Ptr jsAObj = jsArr->getObject( index );

                                HNCObj *aobjPtr;
                                listPtr->appendObj( &aobjPtr );

                                for( pjs::Object::ConstIterator aoit = jsAObj->begin(); aoit != jsAObj->end(); aoit++ )
                                {
                                    if( aoit->second.isString() == true )
                                    {    
                                        aobjPtr->updateValue( aoit->first, aoit->second.extract< std::string >() ); 
                                    }
                                }
                            }
                            else
                            {
                                // Unrecognized config element at array layer
                            }
                        }
                    }
                    else
                    {
                        // Unrecognized config element at section layer.  Log? Error?
                    }
                }
#endif
        }

    }
    catch( Poco::Exception ex )
    {
        std::cerr << "registerEndpointsFromOpenAPI - json error: " << ex.displayText() << std::endl;
        return;
    }

#if 0
    path = ((HNRestHandlerFactory *) m_facPtr)->addPath( dispatchID, "getDeviceInfo",  dispatchInf );
    path->addPathElement( HNRPE_TYPE_PATH, "hnode2" );
    path->addPathElement( HNRPE_TYPE_PATH, "device" );
    path->addPathElement( HNRPE_TYPE_PATH, "info" );

    path = ((HNRestHandlerFactory *) m_facPtr)->addPath( dispatchID, "getDeviceOwner", dispatchInf );
    path->addPathElement( HNRPE_TYPE_PATH, "hnode2" );
    path->addPathElement( HNRPE_TYPE_PATH, "device" );
    path->addPathElement( HNRPE_TYPE_PATH, "owner" );
#endif
}

void
HNHttpServer::start()
{
    std::cout << "Starting HttpServer..." << std::endl;

    pn::ServerSocket svs( m_port );

    m_srvPtr = (void *) new pn::HTTPServer( ((HNRestHandlerFactory *) m_facPtr), svs, new pn::HTTPServerParams );
    ((pn::HTTPServer*)m_srvPtr)->start();
    
     std::cout << "Started HttpServer..." << std::endl; 
}
