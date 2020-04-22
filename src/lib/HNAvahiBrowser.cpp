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

HNAvahiBrowser::HNAvahiBrowser()
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
            // Allocate a Event record
            HNAvahiBrowserEvent *event;
            if( hnaObj->eventQueue.getReleasedCnt() )
                event = (HNAvahiBrowserEvent*) hnaObj->eventQueue.freeRecord();
            else
                event = new HNAvahiBrowserEvent;
            
            // Fill in event data
            event->setName( name );

            // Add it to the event queue
            hnaObj->eventQueue.postRecord( event );
            
#if 0
            char a[AVAHI_ADDRESS_STR_MAX], *t;
            fprintf( stderr, "Service '%s' of type '%s' in domain '%s':\n", name, type, domain );
            avahi_address_snprint( a, sizeof(a), address );
            t = avahi_string_list_to_string( txt );
            fprintf(stderr,
                    "\t%s:%u (%s)\n"
                    "\tTXT=%s\n"
                    "\tcookie is %u\n"
                    "\tis_local: %i\n"
                    "\tour_own: %i\n"
                    "\twide_area: %i\n"
                    "\tmulticast: %i\n"
                    "\tcached: %i\n",
                    host_name, port, a,
                    t,
                    avahi_string_list_get_service_cookie( txt ),
                    !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
                    !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
                    !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
                    !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
                    !!(flags & AVAHI_LOOKUP_RESULT_CACHED));
            avahi_free( t );
#endif
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
            fprintf( stderr, "(Browser) NEW: service '%s' of type '%s' in domain '%s'\n", name, type, domain );
            // We ignore the returned resolver object. In the callback
            //   function we free it. If the server is terminated before
            //   the callback function is called the server will free
            //   the resolver for us.
            resolver = avahi_service_resolver_new( (AvahiClient *) hnaObj->client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags) 0, (AvahiServiceResolverCallback) HNAvahiBrowser::callbackResolve, hnaObj );
            if( resolver == NULL )
                fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror( avahi_client_errno( (AvahiClient *) hnaObj->client ) ) );
        break;

        case AVAHI_BROWSER_REMOVE:
            fprintf( stderr, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n", name, type, domain );
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
                                                "_http._tcp", NULL, (AvahiLookupFlags) 0, 
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
HNAvahiBrowserEvent::setName( std::string value )
{
    name = value;
}

std::string 
HNAvahiBrowserEvent::getName()
{
    return name;
}

