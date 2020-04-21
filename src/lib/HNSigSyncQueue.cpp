#include <unistd.h>
#include <sys/eventfd.h>

#include "HNSigSyncQueue.h"

template <class T>
HNSigSyncQueue<T>::HNSigSyncQueue()
{
    eventFD    = (-1);
    postCnt    = 0;
    releaseCnt = 0;
}

template <class T>
HNSigSyncQueue<T>::~HNSigSyncQueue()
{

}

template <class T>
bool 
HNSigSyncQueue<T>::init()
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

template <class T>
int 
HNSigSyncQueue<T>::getEventFD()
{
    const std::lock_guard< std::mutex > lock( lm );

    return eventFD;
}

template <class T>
uint64_t 
HNSigSyncQueue<T>::getPostedCnt()
{
   // Grab scope lock
   const std::lock_guard< std::mutex > lock( lm );

   // Return count
   return postCnt;
}

template <class T>
void 
HNSigSyncQueue<T>::postRecord( T record )
{
    uint64_t inc = 1;

    // Grab scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Add new record
    postQueue.append_back( record );

    // Account for new entry
    postCnt += 1;

    // Update wakeup event
    ssize_t result = write( eventFD, &inc, sizeof( inc ) );

    if( result != sizeof( inc ) )
    {
        // Error
    }
}

template <class T>
T 
HNSigSyncQueue<T>::aquireRecord()
{
    T rtnObj;
    uint64_t buf;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Bounds check
    if( postCnt == 0 )
        return rtnObj;

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

template <class T>
uint64_t 
HNSigSyncQueue<T>::getReleasedCnt()
{
   // Grab scope lock
   const std::lock_guard< std::mutex > lock( lm );

   // Return count
   return releaseCnt;
}


template <class T>
void 
HNSigSyncQueue<T>::releaseRecord( T record )
{
    // Grab scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Add new record
    releaseQueue.append_back( record );

    // Account for new entry
    releaseCnt += 1;
}

template <class T>
T 
HNSigSyncQueue<T>::freeRecord()
{
    T rtnObj;
    uint64_t buf;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( lm );

    // Bounds check
    if( releaseCnt == 0 )
        return rtnObj;
   
    // Grab the front element from the queue
    rtnObj = releaseQueue.front();
    releaseQueue.pop_front();

    // One less item
    releaseCnt -= 1;

    // Return the object
    return rtnObj; 
}


