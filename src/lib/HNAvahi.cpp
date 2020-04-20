#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>

#include "Poco/Thread.h"
#include "Poco/Runnable.h"

#include <iostream>

#include "HNAvahi.h"

// Helper class for running avahi 
// event loop as an independent thread
class HNAvahiRunner : public Poco::Runnable
{
    private:
        Poco::Thread  thread;
        HNAvahi      *avObj;

    public:  
        HNAvahiRunner( HNAvahi *value )
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

HNAvahi::HNAvahi()
{
    state = HNAVAHI_STATE_IDLE;
    failReason = HNAVAHI_ERR_NONE;

    servicePort = 0;

    client     = NULL; // AvahiClient
    group      = NULL; // AvahiEntryGroup
    simplePoll = NULL; // AvahiSimplePoll 
    thelp      = NULL; // HNAvahiRunner
}

HNAvahi::~HNAvahi()
{
    cleanup();
}

void
HNAvahi::setID( std::string srvType, std::string srvName )
{
    if( srvType.empty() == false )
        serviceType = avahi_strdup( srvType.c_str() );
    else
        serviceType = NULL;

    if( srvName.empty() == false )
        serviceName = avahi_strdup( srvName.c_str() );
    else
        serviceName = NULL;
}
 
void
HNAvahi::setPort( uint16_t port )
{
    servicePort = port;
}

std::string 
HNAvahi::getSrvType()
{
    std::string result;

    if( serviceType != NULL )
        result = serviceType;

    return result;
}

std::string 
HNAvahi::getSrvName()
{
    std::string result;

    if( serviceName != NULL )
        result = serviceName;

    return result;
}

void 
HNAvahi::setSrvPair( std::string key, std::string value )
{
    if( key.empty() == true )
        return;

    srvPairs[ key ] = value;
}

void 
HNAvahi::setSrvTag( std::string key )
{
    std::string empty;

    if( key.empty() == true )
        return;

    srvPairs[ key ] = empty;
}

void 
HNAvahi::clearSrvPairs()
{
    srvPairs.clear();
}

void
HNAvahi::cleanup()
{
    if( client )
        avahi_client_free( (AvahiClient*) client );

    if( simplePoll )
        avahi_simple_poll_free( (AvahiSimplePoll*) simplePoll );

    if( thelp )
        delete ( (HNAvahiRunner*) thelp );

    client = NULL;
    group = NULL; 
    simplePoll = NULL;  
    thelp = NULL; 

    state = HNAVAHI_STATE_IDLE;
    failReason = HNAVAHI_ERR_NONE;
}

void 
HNAvahi::setState( HNAVAHI_STATE_T value )
{
    state = value;
}

void
HNAvahi::setFailure( HNAVAHI_ERROR_T reason )
{
    failReason = reason;
    failMsg = avahi_strerror( avahi_client_errno( (AvahiClient*) client ) );

    fprintf( stderr, "Client failure( %d ): %s\n", reason, failMsg.c_str() );
}

void
HNAvahi::setGroup( void *value )
{
    group = value;
}

bool
HNAvahi::hasGroup()
{
    return ( (group == NULL) ? false : true );
}

void
HNAvahi::resetGroup()
{
    avahi_entry_group_reset( (AvahiEntryGroup*) group );
}

void
HNAvahi::setClient( void *value )
{
    client = value;
}

bool
HNAvahi::hasClient()
{
    return ( (client == NULL) ? false : true );
}

void 
HNAvahi::createService() 
{
    int ret;

    // If this is the first time we're called, let's create a new
    // entry group if necessary
    if( !group )
    {
        std::cout << "HNAvahi::client: " << client << std::endl;

        group = (void*) avahi_entry_group_new( (AvahiClient*) client, (AvahiEntryGroupCallback) HNAvahi::callbackEntryGroup, this );
        if( !group )
        {
            //fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(c)));
            setFailure( HNAVAHI_ERR_GRPNEW );
            goto fail;
        }
    }

    // If the group is empty (either because it was just created, or
    // because it was reset previously, add our entries.
    if( avahi_entry_group_is_empty( (AvahiEntryGroup*) group ) ) 
    {
        fprintf(stderr, "Adding service '%s'\n", serviceName);

        // Create a string array of the TXT data
        AvahiStringList *txtList = avahi_string_list_new( NULL, NULL );

        for( std::map< std::string, std::string >::iterator it = srvPairs.begin(); it != srvPairs.end(); it++ )
        {
            if( it->second.empty() == true )
                txtList = avahi_string_list_add( txtList, it->first.c_str() );
            else
                txtList = avahi_string_list_add_printf( txtList, "%s=%s", it->first.c_str(), it->second.c_str() );
        }

        // We will now add two services and one subtype to the entry
        // group. The two services have the same name, but differ in
        // the service type (IPP vs. BSD LPR). Only services with the
        // same name should be put in the same entry group.
        // Add the service for IPP 
        ret = avahi_entry_group_add_service_strlst( (AvahiEntryGroup*) group, 
                                             AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags) 0, 
                                             serviceName, serviceType, 
                                             NULL, NULL, servicePort, txtList );

        avahi_string_list_free( txtList );

        if( ret < 0) 
        {
            if( ret == AVAHI_ERR_COLLISION )
                setFailure( HNAVAHI_ERR_COLLISION );
                goto fail;

            //fprintf(stderr, "Failed to add _ipp._tcp service: %s\n", avahi_strerror(ret));
            setFailure( HNAVAHI_ERR_SRVADD );
            goto fail;
        }

        // Tell the server to register the service
        ret = avahi_entry_group_commit( (AvahiEntryGroup*) group );
        if( ret < 0 ) 
        {
            //fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
            setFailure( HNAVAHI_ERR_GRPCOMMIT );
            goto fail;
        }
    }

    return;

fail:
    shutdown();
}

void 
HNAvahi::callbackEntryGroup( void *grpval, int stval, void *userdata ) 
{
    std::cout << "HNAvahi::callbackEntryGroup()" << std::endl;

    HNAvahi *hnaObj = (HNAvahi *) userdata;
    AvahiClientState state = (AvahiClientState) stval;

    hnaObj->setGroup( grpval );

    // Called whenever the entry group state changes 
    switch( state ) 
    {
        case AVAHI_ENTRY_GROUP_ESTABLISHED:
            // The entry group has been established successfully
            std::cout << "Service established" << std::endl;
            //fprintf(stderr, "Service '%s' successfully established.\n", serviceName);
            hnaObj->setState( HNAVAHI_STATE_ESTABLISHED );
        break;

        case AVAHI_ENTRY_GROUP_COLLISION: 
        {
            // A service name collision with a remote service
            // happened. Set the error.
            hnaObj->setFailure( HNAVAHI_ERR_COLLISION );
            hnaObj->shutdown();
        }
        break;

        case AVAHI_ENTRY_GROUP_FAILURE:
        {
            //fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));
            hnaObj->setFailure( HNAVAHI_ERR_GRPFAILURE );
            // Some kind of failure happened while we were registering our services
            hnaObj->shutdown();
        }
        break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
        break;
    }
}

void 
HNAvahi::callbackClient( void *c, int stval, void *userdata ) 
{
    std::cout << "HNAvahi::callbackClient()" << std::endl;

    HNAvahi *hnaObj  = (HNAvahi *) userdata;
    AvahiClientState state = (AvahiClientState) stval;

    // Assign the new client object
    hnaObj->setClient( c );

    // Called whenever the client or server state changes 
    switch( state ) 
    {
        // The server has startup successfully and registered its host
        // name on the network, so it's time to create our services
        case AVAHI_CLIENT_S_RUNNING:
            hnaObj->createService();
        break;

        case AVAHI_CLIENT_FAILURE:
            hnaObj->setFailure( HNAVAHI_ERR_CLTFAIL );
            hnaObj->shutdown(); 
        break;

        // Let's drop our registered services. When the server is back
        // in AVAHI_SERVER_RUNNING state we will register them
        // again with the new host name.
        // The server records are now being established. This
        // might be caused by a host name change. We need to wait
        // for our own records to register until the host name is
        // properly esatblished.
        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_S_REGISTERING:
            if( hnaObj->hasGroup() )
                hnaObj->resetGroup(); 
        break;

        case AVAHI_CLIENT_CONNECTING:
        break;
    }
}

void
HNAvahi::start()
{
    int error;

    std::cout << "HNAvahi::start()" << std::endl;

    // Signal in startup
    setState( HNAVAHI_STATE_STARTUP );

    // Allocate main loop object
    simplePoll = avahi_simple_poll_new();
    std::cout << "HNAvahi::start - simplePoll: " << simplePoll << std::endl;
    if( !simplePoll )
    {
        //fprintf(stderr, "Failed to create simple poll object.\n");
        setFailure( HNAVAHI_ERR_POLLCREATE );
        cleanup();
        return;
    }

    // Allocate a new client.
    client = avahi_client_new( avahi_simple_poll_get( (AvahiSimplePoll*) simplePoll ), 
                               (AvahiClientFlags) 0, 
                               (AvahiClientCallback) HNAvahi::callbackClient, this,
                               &error);

    std::cout << "HNAvahi::start - client: " << client << std::endl;
    if( !client )
    {
        //fprintf (stderr, "Failed to create client: %s\n", avahi_strerror( error ) );
        setFailure( HNAVAHI_ERR_CLTCREATE );
        cleanup();
        return;
    }

    // Allocate the thread helper
    thelp = new HNAvahiRunner( this );
    if( !thelp )
    {
        setFailure( HNAVAHI_ERR_RUNCREATE );
        cleanup();
        return;
    }

    // Start up the event loop
    ( (HNAvahiRunner*) thelp )->startThread();
}

void 
HNAvahi::runEventLoop()
{
    std::cout << "HNAvahi::runEventLoop()" << std::endl;

    // Run the main loop
    avahi_simple_poll_loop( (AvahiSimplePoll*) simplePoll );
}

void
HNAvahi::shutdown()
{
    setState( HNAVAHI_STATE_SHUTDOWN );

    if( !thelp )
    {
        cleanup();
        return;
    }

    // End the event loop
    ( (HNAvahiRunner*) thelp )->killThread();
}

void 
HNAvahi::killEventLoop()
{
    avahi_simple_poll_quit( (AvahiSimplePoll*) simplePoll );
}


