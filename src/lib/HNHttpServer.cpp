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
    std::vector< std::string > pathStrs;

    Poco::URI uri( request.getURI() );    

    //if (request.getURI() == "/")
    //    return new HNodeRestHandler();
    //else

    //std::cout << "Server Request method: " << request.getMethod() << std::endl;
    //std::cout << "Server Request URI: " << uri.toString() << std::endl;

    uri.getPathSegments( pathStrs );

    for( std::vector< HNRestPath >::iterator it = pathList.begin(); it != pathList.end(); it++ )
    {
        //std::cout << "Check handler: " << it->getOpID() << std::endl;
        opData = it->checkForHandler( request.getMethod(), pathStrs );
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
        // Attempt to parse the provided openAPI input
        pjs::Parser parser;
        pdy::Var varRoot = parser.parse( openAPIJson );

        // Get a pointer to the root object
        pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

        // Make sure the "paths" field exists, otherwise nothing to do
        if( jsRoot->isObject( "paths" ) == false )
            return;

        // Extract the paths object
        pjs::Object::Ptr jsPathList = jsRoot->getObject( "paths" );

        // Iterate through the fields, each field represents a path
        for( pjs::Object::ConstIterator pit = jsPathList->begin(); pit != jsPathList->end(); pit++ )
        {
            std::vector< std::string > pathStrs;

            // Parse the extracted uri
            Poco::URI uri( pit->first );

            // Break it into tokens by '/'
            uri.getPathSegments( pathStrs );

            std::cout << "regend - uri: " << uri.toString() << std::endl;
            std::cout << "regend - segcnt: " << pathStrs.size() << std::endl;

            // Turn the iterator into a object pointer
            pjs::Object::Ptr jsPath = pit->second.extract< pjs::Object::Ptr >();

            // Iterate through the fields, each field represents a supported HTTP Method
            for( pjs::Object::ConstIterator mit = jsPath->begin(); mit != jsPath->end(); mit++ )
            {
                std::cout << "regend - method: " << mit->first << std::endl;

                // Get a pointer to the method object
                pjs::Object::Ptr jsMethod = mit->second.extract< pjs::Object::Ptr >();

                // The method object must contain a operationID field
                // which will be used by the dispatch Callback to know
                // the request that was made.
                std::string opID = jsMethod->getValue< std::string >( "operationId" );
                std::cout << "regend - opID: " << opID << std::endl;

                // We have everything we need so now the new path can be added to the 
                // http factory class
                path = ((HNRestHandlerFactory *) m_facPtr)->addPath( dispatchID, opID,  dispatchInf );

                // Record the method for this specific path record
                path->setMethod( mit->first );

                // Build up the path element array, taking into account url based parameters
                for( std::vector< std::string >::iterator sit = pathStrs.begin(); sit != pathStrs.end(); sit++ )
                {
                    std::cout << "PathComp: " << *sit << std::endl;
                    // Check if this is a parameter or a regular path element.
                    if( (sit->front() == '{') && (sit->back() == '}') )
                    {
                        // Add a parameter capture
                        std::string paramName( (sit->begin() + 1), (sit->end() - 1) );
                        std::cout << "regend - paramName: " << paramName << std::endl;
                        path->addPathElement( HNRPE_TYPE_PARAM, paramName );
                    }
                    else
                    {
                        // Add a regular path element
                        path->addPathElement( HNRPE_TYPE_PATH, *sit );
                    }
                } 
            }
        }

    }
    catch( Poco::Exception ex )
    {
        std::cerr << "registerEndpointsFromOpenAPI - json error: " << ex.displayText() << std::endl;
        return;
    }

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
