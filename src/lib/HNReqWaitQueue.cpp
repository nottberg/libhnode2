#include <unistd.h>
#include <sys/eventfd.h>

#include <iostream>
#include <thread>
#include <chrono>

#include "HNReqWaitQueue.h"

using namespace std::chrono_literals;

HNReqWaitAction::HNReqWaitAction()
{

}

HNReqWaitAction::~HNReqWaitAction()
{

}

HNRW_RESULT_T
HNReqWaitAction::wait()
{
    int idx = 35;

    // Wait until we are woken up or timeout
    std::unique_lock<std::mutex> lk(m);

    resCode = HNRW_RESULT_WAITING;

    std::cerr << "Thread " << std::this_thread::get_id() << " waiting...." << '\n';

    if( cv.wait_for( lk, idx*1000ms, [this]{return resCode != HNRW_RESULT_WAITING;} ) ) 
    {
        std::cerr << "Thread " << std::this_thread::get_id() << " finished waiting. resCode == " << resCode << '\n';
    }
    else
    {
        std::cerr << "Thread " << std::this_thread::get_id() << " timed out. resCode == " << resCode << '\n';
        resCode = HNRW_RESULT_TIMEOUT;
    }

    return resCode;
}

void
HNReqWaitAction::complete()
{
    std::cerr << "Notifying...thread: \n" << std::this_thread::get_id() << std::endl;
    std::lock_guard<std::mutex> lk(m);
    resCode = HNRW_RESULT_SUCCESS;
    cv.notify_all();
}

HNReqWaitQueue::HNReqWaitQueue()
{
    eventFD    = (-1);
    postCnt    = 0;
    releaseCnt = 0;
}

HNReqWaitQueue::~HNReqWaitQueue()
{

}

bool 
HNReqWaitQueue::init()
{
    const std::lock_guard< std::mutex > lock( queueMutex );

    if( eventFD != -1 )
        close( eventFD );

    postCnt = 0;
    postQueue.clear();

    eventFD = eventfd( 0, EFD_NONBLOCK );

    if( eventFD < 0 )
    {
        return true;
    }

    return false;
}

int 
HNReqWaitQueue::getEventFD()
{
    const std::lock_guard< std::mutex > lock( queueMutex );

    return eventFD;
}

uint64_t 
HNReqWaitQueue::getPostedCnt()
{
   // Grab scope lock
   const std::lock_guard< std::mutex > lock( queueMutex );

   // Return count
   return postCnt;
}

void 
HNReqWaitQueue::postAndWait( HNReqWaitAction *record )
{
    uint64_t inc = 1;

    std::cout << "postAndWait...start" << std::endl;

    // Grab queue lock
    std::unique_lock< std::mutex > qlock( queueMutex );

    //std::cout << "postAndWait...locking queue" << std::endl;

    //qlock.lock();

    std::cout << "postAndWait...queue" << std::endl;

    // Add new record
    postQueue.push_back( record );

    std::cout << "postAndWait...cnt" << std::endl;

    // Account for new entry
    postCnt += 1;

    std::cout << "postAndWait...eventFD" << std::endl;

    // Update wakeup event
    ssize_t result = write( eventFD, &inc, sizeof( inc ) );

    if( result != sizeof( inc ) )
    {
        // Error
    }

    qlock.unlock();

    std::cout << "Waiting..." << std::endl;

    record->wait();
}

HNReqWaitAction*
HNReqWaitQueue::aquireRecord()
{
    HNReqWaitAction *rtnObj;
    uint64_t buf;

    // Grab the scope lock
    const std::lock_guard< std::mutex > lock( queueMutex );

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

