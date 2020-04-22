#include <unistd.h>
#include <sys/eventfd.h>

#include <iostream>

#include "HNSigSyncQueue.h"

HNSigSyncQueue::HNSigSyncQueue()
{
    eventFD    = (-1);
    postCnt    = 0;
    releaseCnt = 0;
}

HNSigSyncQueue::~HNSigSyncQueue()
{

}

bool 
HNSigSyncQueue::init()
{
    const std::lock_guard< std::mutex > lock( lm );

    if( eventFD != -1 )
        close( eventFD );

    postCnt = 0;
    postQueue.clear();

    releaseCnt = 0;
    releaseQueue.clear();

    eventFD = eventfd( 0, EFD_NONBLOCK );

    if( eventFD < 0 )
    {
        return true;
    }

    return false;
}

int 
HNSigSyncQueue::getEventFD()
{
    const std::lock_guard< std::mutex > lock( lm );

    return eventFD;
}

uint64_t 
HNSigSyncQueue::getPostedCnt()
{
   // Grab scope lock
   const std::lock_guard< std::mutex > lock( lm );

   // Return count
   return postCnt;
}

void 
HNSigSyncQueue::postRecord( void *record )
{
    uint64_t inc = 1;

    // Grab scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Add new record
    postQueue.push_back( record );

    // Account for new entry
    postCnt += 1;

    // Update wakeup event
    ssize_t result = write( eventFD, &inc, sizeof( inc ) );

    if( result != sizeof( inc ) )
    {
        // Error
    }
}

void*
HNSigSyncQueue::aquireRecord()
{
    void *rtnObj;
    uint64_t buf;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Bounds check
    if( postCnt == 0 )
        return NULL;

    // Read the eventFD, which will clear it to zero
    read( eventFD, &buf, sizeof(buf) ); 
    
    // Grab the front element from the queue
    rtnObj = postQueue.front();
    postQueue.pop_front();

    // One less item
    postCnt -= 1;

    // Return the object
    return rtnObj; 
}

uint64_t 
HNSigSyncQueue::getReleasedCnt()
{
   // Grab scope lock
   const std::lock_guard< std::mutex > lock( lm );

   // Return count
   return releaseCnt;
}


void 
HNSigSyncQueue::releaseRecord( void *record )
{
    // Grab scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Add new record
    releaseQueue.push_back( record );

    // Account for new entry
    releaseCnt += 1;
}

void*
HNSigSyncQueue::freeRecord()
{
    void *rtnObj;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Bounds check
    if( releaseCnt == 0 )
        return NULL;
   
    // Grab the front element from the queue
    rtnObj = releaseQueue.front();
    releaseQueue.pop_front();

    // One less item
    releaseCnt -= 1;

    // Return the object
    return rtnObj; 
}


