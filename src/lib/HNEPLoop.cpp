#include <iostream>

#include <sys/eventfd.h>

#include "HNEPLoop.h"

#define MAXEVENTS 8

HNEPTrigger::HNEPTrigger()
{
    m_fd = 0;
}

HNEPTrigger::~HNEPTrigger()
{
    if( m_fd != 0 )
        close( m_fd );
    m_fd = 0;
}

HNEP_RESULT_T 
HNEPTrigger::setup()
{
    m_fd = eventfd( 0, 0 );
    if( m_fd == -1 )
        return HNEP_RESULT_FAILURE;

    std::cout << "HNEPTrigger - new trigger fd: " << m_fd << std::endl;

    return HNEP_RESULT_SUCCESS;
}

void
HNEPTrigger::clear()
{
    if( m_fd != 0 )
        close( m_fd );
    m_fd = 0;
}

uint 
HNEPTrigger::getFD()
{
    return m_fd;
}

void 
HNEPTrigger::trigger()
{
    uint64_t value = 1;
    ssize_t result;

    if( m_fd == 0 )
        return;

    result = write( m_fd, &value, sizeof(value) );

    std::cout << "HNEPTrigger::trigger - fd: " << m_fd << "  result: " << result << std::endl;
}

bool 
HNEPTrigger::isMatch( uint sfd )
{
    return (m_fd == sfd);
}

void 
HNEPTrigger::reset()
{
    uint64_t value = 1;
    ssize_t result;

    if( m_fd == 0 )
        return;

    result = read( m_fd, &value, sizeof(value) );
    
    std::cout << "HNEPTrigger::reset - fd: " << m_fd << "  result: " << result << std::endl;
}

HNEPLoop::HNEPLoop()
{
    m_quit = true;
    m_epollFD   = (-1);
    m_events    = NULL;
    m_timeoutMS = 2000;
}

HNEPLoop::~HNEPLoop()
{
    if( m_epollFD != (-1) )
    {
        close( m_epollFD );
        m_epollFD = (-1);
    }

    if( m_events != NULL )
    {
        free( m_events );
        m_events = NULL;
    }
}

HNEP_RESULT_T 
HNEPLoop::setup( HNEPLoopCallbacks *parent )
{
    // Set callbacks
    m_parent = parent;

    // Initialize for event loop
    m_epollFD = epoll_create1( 0 );
    if( m_epollFD == -1 )
    {
        //log.error( "ERROR: Failure to create epoll event loop: %s", strerror(errno) );
        return HNEP_RESULT_FAILURE;
    }

    // Buffer where events are returned 
    m_events = (struct epoll_event *) calloc( MAXEVENTS, sizeof( struct epoll_event ) );
    if( m_events == NULL )
    {
        //log.error( "ERROR: Failed to allocate event space: %s", strerror(errno) );
        return HNEP_RESULT_FAILURE;
    }

    // Ready to run again.
    m_quit = false;

    // Setup complete
    return HNEP_RESULT_SUCCESS;
}

void 
HNEPLoop::stop()
{
    m_quit = true;
}

HNEP_RESULT_T
HNEPLoop::run()
{
    if( m_quit == true )
    {
        //log.error( "Event Loop not initialized" );
        return HNEP_RESULT_FAILURE;
    }

    // The event loop 
    while( m_quit == false )
    {
        int n;
        int i;

        // Check for events
        n = epoll_wait( m_epollFD, m_events, MAXEVENTS, m_timeoutMS );

        // EPoll error
        if( n < 0 )
        {
            // If we've been interrupted by an incoming signal, continue, wait for socket indication
            if( errno == EINTR )
                continue;

            // Handle error
            //log.error( "ERROR: Failure report by epoll event loop: %s", strerror( errno ) );
            return HNEP_RESULT_FAILURE;
        }

        // Check these critical tasks everytime
        // the event loop wakes up.
        // ADD CALLBACK
        m_parent->loopIteration();

        // If it was a timeout then continue to next loop
        // skip socket related checks.
        if( n == 0 )
        {
            // ADD CALLBACK
            m_parent->timeoutEvent();
            continue;
        }

        // Socket event
        for( i = 0; i < n; i++ )
	    {
            // ADD CALLBACK
            if( ( m_events[i].events & EPOLLERR ) || ( m_events[i].events & EPOLLHUP ) )
                m_parent->fdError( m_events[i].data.fd );
            else
                m_parent->fdEvent( m_events[i].data.fd );
        }
    }

    // Successful exit
    return HNEP_RESULT_SUCCESS;
}

HNEP_RESULT_T
HNEPLoop::setupTriggerFD( HNEPTrigger &trigger )
{
    if( trigger.setup() != HNEP_RESULT_SUCCESS )
        return HNEP_RESULT_FAILURE;
    
    return addFDToEPoll( trigger.getFD() );
}

void
HNEPLoop::removeTriggerFD( HNEPTrigger &trigger )
{
    removeFDFromEPoll( trigger.getFD() );
    trigger.clear();
}

HNEP_RESULT_T
HNEPLoop::addFDToEPoll( int sfd )
{
    struct epoll_event event;
    int flags, s;

    flags = fcntl( sfd, F_GETFL, 0 );
    if( flags == -1 )
    {
        //log.error( "Failed to get socket flags: %s", strerror(errno) );
        return HNEP_RESULT_FAILURE;
    }

    flags |= O_NONBLOCK;
    s = fcntl( sfd, F_SETFL, flags );
    if( s == -1 )
    {
        //log.error( "Failed to set socket flags: %s", strerror(errno) );
        return HNEP_RESULT_FAILURE; 
    }

    event.data.fd = sfd;
    event.events = EPOLLIN;
    s = epoll_ctl( m_epollFD, EPOLL_CTL_ADD, sfd, &event );
    if( s == -1 )
    {
        return HNEP_RESULT_FAILURE;
    }

    return HNEP_RESULT_SUCCESS;
}

HNEP_RESULT_T
HNEPLoop::removeFDFromEPoll( int sfd )
{
    int s;

    s = epoll_ctl( m_epollFD, EPOLL_CTL_DEL, sfd, NULL );
    if( s == -1 )
    {
        return HNEP_RESULT_FAILURE;
    }

    return HNEP_RESULT_SUCCESS;
}

