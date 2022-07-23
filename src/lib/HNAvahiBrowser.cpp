#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "Poco/Thread.h"
#include "Poco/Runnable.h"

#include <iostream>

#include "HNAvahiBrowser.h"

// Helper class for running avahi 
// event loop as an independent thread
class HNAvahiBrowserRunner : public Poco::Runnable
{
    private:
        Poco::Thread   thread;
        HNAvahiBrowser *avObj;

    public:  
        HNAvahiBrowserRunner( HNAvahiBrowser *value )
        {
            avObj = value;
        }

        void startThread()
        {
            thread.start( *this );
        }

        void killThread()
        {
            avObj->killEventLoop();
            thread.join();
        }

        virtual void run()
        {
            avObj->runEventLoop();
        }

};

HNAvahiBrowser::HNAvahiBrowser( std::string avahiType )
: devType( avahiType )
{
    state = HNAVAHI_BROWSER_STATE_IDLE;
    failReason = HNAVAHI_BROWSER_ERR_NONE;

    client         = NULL; // AvahiClient
    serviceBrowser = NULL; // AvahiServiceBrowser
    simplePoll     = NULL; // AvahiSimplePoll 
    thelp          = NULL; // HNAvahiRunner
}

HNAvahiBrowser::~HNAvahiBrowser()
{
    cleanup();
}

HNSigSyncQueue& 
HNAvahiBrowser::getEventQueue()
{
    return eventQueue;
}

void
HNAvahiBrowser::cleanup()
{
    if( serviceBrowser )
        avahi_service_browser_free( (AvahiServiceBrowser *) serviceBrowser );

    if( client )
        avahi_client_free( (AvahiClient*) client );

    if( simplePoll )
        avahi_simple_poll_free( (AvahiSimplePoll*) simplePoll );

    if( thelp )
        delete ( (HNAvahiBrowserRunner*) thelp );

    client = NULL;
    serviceBrowser = NULL; 
    simplePoll = NULL;  
    thelp = NULL; 

    state = HNAVAHI_BROWSER_STATE_IDLE;
    failReason = HNAVAHI_BROWSER_ERR_NONE;
}

void 
HNAvahiBrowser::setState( HNAVAHI_BROWSER_STATE_T value )
{
    state = value;
}

void
HNAvahiBrowser::setFailure( HNAVAHI_BROWSER_ERROR_T reason )
{
    failReason = reason;
    failMsg = avahi_strerror( avahi_client_errno( (AvahiClient*) client ) );

    fprintf( stderr, "Client failure( %d ): %s\n", reason, failMsg.c_str() );
}

void
HNAvahiBrowser::setClient( void *value )
{
    client = value;
}

bool
HNAvahiBrowser::hasClient()
{
    return ( (client == NULL) ? false : true );
}

void 
HNAvahiBrowser::callbackResolve( void *r, uint interface, uint protocol, uint event, 
                                 const char *name,  const char *type, const char *domain, 
                                 const char *host_name, const void *addr, uint16_t port, 
                                 void *tsl, uint flags, void* userdata)
{
    HNAvahiBrowser *hnaObj = (HNAvahiBrowser *) userdata;
    AvahiServiceResolver *resolver = (AvahiServiceResolver *) r;
    AvahiAddress *address = (AvahiAddress *) addr;
    AvahiStringList *txt = (AvahiStringList *) tsl;

    // Called whenever a service has been resolved successfully or timed out
    switch( event ) 
    {
        case AVAHI_RESOLVER_FAILURE:
            fprintf( stderr, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror( avahi_client_errno( (AvahiClient *) hnaObj->client ) ) );
        break;

        case AVAHI_RESOLVER_FOUND: 
        {
            char a[AVAHI_ADDRESS_STR_MAX];

            // Allocate an Event record
            HNAvahiBrowserEvent *event;
            if( hnaObj->eventQueue.getReleasedCnt() )
                event = (HNAvahiBrowserEvent*) hnaObj->eventQueue.freeRecord();
            else
                event = new HNAvahiBrowserEvent;
            
            // Fill in event data
            avahi_address_snprint( a, sizeof(a), address );
            event->setAddEvent( name, type, domain, host_name, a, port );

            while( txt )
            {
                char* key;
                char* value;
                std::size_t size;
                std::string valueStr;

                avahi_string_list_get_pair( txt, &key, &value, &size );

                if( value != NULL )
                     valueStr.assign( value, size );

                event->addTxtPair( key, valueStr );

                avahi_free(key);
                avahi_free(value);
                txt = avahi_string_list_get_next( txt );
            }

            // Add it to the event queue
            hnaObj->eventQueue.postRecord( event );
        }
    }
    avahi_service_resolver_free( resolver );
}


void 
HNAvahiBrowser::callbackBrowse( void *b, uint interface, uint protocol, uint event, 
                                const char *name, const char *type, const char *domain, 
                                uint flags, void* userdata )
{
    HNAvahiBrowser *hnaObj = (HNAvahiBrowser *) userdata;
    AvahiServiceResolver *resolver = NULL;

    // Called whenever a new services becomes available on the LAN or is removed from the LAN
    switch( event ) 
    {
        case AVAHI_BROWSER_FAILURE:
            fprintf( stderr, "(Browser) %s\n", avahi_strerror( avahi_client_errno( (AvahiClient *) hnaObj->client ) ) );
            hnaObj->setFailure( HNAVAHI_BROWSER_ERR_CLTFAIL );
            hnaObj->shutdown(); 
            return;
        break;

        case AVAHI_BROWSER_NEW:
            //fprintf( stderr, "(Browser) NEW: service '%s' of type '%s' in domain '%s'\n", name, type, domain );
            // We ignore the returned resolver object. In the callback
            //   function we free it. If the server is terminated before
            //   the callback function is called the server will free
            //   the resolver for us.
            resolver = avahi_service_resolver_new( (AvahiClient *) hnaObj->client, interface, protocol,
                                                    name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags) 0,
                                                   (AvahiServiceResolverCallback) HNAvahiBrowser::callbackResolve, hnaObj );

            if( resolver == NULL )
                fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror( avahi_client_errno( (AvahiClient *) hnaObj->client ) ) );
        break;

        case AVAHI_BROWSER_REMOVE:
        {
            // Allocate an Event record
            HNAvahiBrowserEvent *event;
            if( hnaObj->eventQueue.getReleasedCnt() )
                event = (HNAvahiBrowserEvent*) hnaObj->eventQueue.freeRecord();
            else
                event = new HNAvahiBrowserEvent;
            
            // Fill in event data
            event->setRemoveEvent( name, type, domain );

            // Add it to the event queue
            hnaObj->eventQueue.postRecord( event );
        }
        break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            fprintf( stderr, "(Browser) %s\n", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW" );
        break;
    }
}

void 
HNAvahiBrowser::callbackClient( void *c, int stval, void *userdata ) 
{
    std::cout << "HNAvahiBrowser::callbackClient()" << std::endl;

    HNAvahiBrowser *hnaObj = (HNAvahiBrowser *) userdata;
    AvahiClientState state = (AvahiClientState) stval;

    // Assign the new client object
    hnaObj->setClient( c );

    // Called whenever the client or server state changes 
    switch( state ) 
    {
        case AVAHI_CLIENT_FAILURE:
            hnaObj->setFailure( HNAVAHI_BROWSER_ERR_CLTFAIL );
            hnaObj->shutdown(); 
        break;
    }
}

void
HNAvahiBrowser::start()
{
    int error;

    std::cout << "HNAvahiBrowser::start()" << std::endl;

    // Signal in startup
    setState( HNAVAHI_BROWSER_STATE_STARTUP );

    // Init the event queue for sending back discover events.
    eventQueue.init();

    // Allocate main loop object
    simplePoll = avahi_simple_poll_new();
    std::cout << "HNAvahiBrowser::start - simplePoll: " << simplePoll << std::endl;
    if( simplePoll == NULL )
    {
        //fprintf(stderr, "Failed to create simple poll object.\n");
        setFailure( HNAVAHI_BROWSER_ERR_POLLCREATE );
        cleanup();
        return;
    }

    // Allocate a new client.
    client = avahi_client_new( avahi_simple_poll_get( (AvahiSimplePoll*) simplePoll ), 
                               (AvahiClientFlags) 0, 
                               (AvahiClientCallback) HNAvahiBrowser::callbackClient, this,
                               &error);

    std::cout << "HNAvahiBrowser::start - client: " << client << std::endl;
    if( client == NULL )
    {
        //fprintf (stderr, "Failed to create client: %s\n", avahi_strerror( error ) );
        setFailure( HNAVAHI_BROWSER_ERR_CLTCREATE );
        cleanup();
        return;
    }

    // Allocate a service browser
    serviceBrowser = avahi_service_browser_new( (AvahiClient *) client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 
                                                devType.c_str(), NULL, (AvahiLookupFlags) 0, 
                                                (AvahiServiceBrowserCallback) HNAvahiBrowser::callbackBrowse, this );

    if( serviceBrowser == NULL )
    {
        fprintf( stderr, "Failed to create service browser: %s\n", avahi_strerror( avahi_client_errno( (AvahiClient *) client ) ) );
        setFailure( HNAVAHI_BROWSER_ERR_CLTCREATE );
        cleanup();
        return;
    }

    // Allocate the thread helper
    thelp = new HNAvahiBrowserRunner( this );
    if( !thelp )
    {
        setFailure( HNAVAHI_BROWSER_ERR_RUNCREATE );
        cleanup();
        return;
    }

    // Start up the event loop
    ( (HNAvahiBrowserRunner*) thelp )->startThread();
}

void 
HNAvahiBrowser::runEventLoop()
{
    std::cout << "HNAvahiBrowser::runEventLoop()" << std::endl;

    // Run the main loop
    avahi_simple_poll_loop( (AvahiSimplePoll*) simplePoll );
}

void
HNAvahiBrowser::shutdown()
{
    setState( HNAVAHI_BROWSER_STATE_SHUTDOWN );

    if( !thelp )
    {
        cleanup();
        return;
    }

    // End the event loop
    ( (HNAvahiBrowserRunner*) thelp )->killThread();
}

void 
HNAvahiBrowser::killEventLoop()
{
    avahi_simple_poll_quit( (AvahiSimplePoll*) simplePoll );
}


HNAvahiBrowserEvent::HNAvahiBrowserEvent()
{

}

HNAvahiBrowserEvent::~HNAvahiBrowserEvent()
{

}

void 
HNAvahiBrowserEvent::clear()
{
    evType = HNAB_EVTYPE_NOTSET;

    name.clear();
    type.clear();
    domain.clear();

    hostname.clear();
    address.clear();
    port = 0;

    txtPairs.clear();
}

void 
HNAvahiBrowserEvent::setAddEvent( std::string nm, std::string tp, std::string dm, std::string hn, std::string ad, uint16_t pt )
{
    clear();

    evType = HNAB_EVTYPE_ADD;
    name   = nm;
    type   = tp;
    domain = dm;

    hostname = hn;
    address  = ad;
    port     = pt;

}

void 
HNAvahiBrowserEvent::setRemoveEvent( std::string nm, std::string tp, std::string dm )
{
    clear();

    evType = HNAB_EVTYPE_REMOVE;
    name   = nm;
    type   = tp;
    domain = dm;
}

void 
HNAvahiBrowserEvent::addTxtPair( std::string key, std::string value )
{
    txtPairs.insert( std::pair< std::string, std::string >( key, value ) );
}

void 
HNAvahiBrowserEvent::setName( std::string value )
{
    name = value;
}

HNAB_EVTYPE_T 
HNAvahiBrowserEvent::getEventType()
{
    return evType;
}

std::string 
HNAvahiBrowserEvent::getName()
{
    return name;
}

std::string 
HNAvahiBrowserEvent::getTxtValue( std::string key )
{
    std::string empty;

    std::map< std::string, std::string >::iterator it = txtPairs.find( key );

    if( it == txtPairs.end() )
        return empty;

    return it->second;    
}

std::string 
HNAvahiBrowserEvent::getHostname()
{
    return hostname;
}

std::string 
HNAvahiBrowserEvent::getAddress()
{
    return address;
}

uint16_t    
HNAvahiBrowserEvent::getPort()
{
    return port;
}

void 
HNAvahiBrowserEvent::debugPrint()
{

    if( evType == HNAB_EVTYPE_ADD )
    {
        printf( "ADD Service '%s' of type '%s' in domain '%s':\n", name.c_str(), type.c_str(), domain.c_str() );
        printf( "\t%s:%u (%s)\n", hostname.c_str(), port, address.c_str() );
        
        for( std::map< std::string, std::string >::iterator it = txtPairs.begin(); it != txtPairs.end(); it++ )
        {
            printf( "\t%s = %*.*s\n", it->first.c_str(), (int) it->second.size(), (int) it->second.size(), it->second.c_str() );
        }    
    }
    else if( evType == HNAB_EVTYPE_REMOVE )
    {
        printf( "REMOVE Service '%s' of type '%s' in domain '%s':\n", name.c_str(), type.c_str(), domain.c_str() );
    }

}


