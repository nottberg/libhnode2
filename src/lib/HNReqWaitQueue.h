#ifndef _HN_REQ_WAIT_QUEUE_H_
#define _HN_REQ_WAIT_QUEUE_H_

#include <list>
#include <mutex>
#include <condition_variable>

typedef enum HNReqWaitActionResultEnum
{
    HNRW_RESULT_SUCCESS,
    HNRW_RESULT_FAILURE,
    HNRW_RESULT_WAITING,
    HNRW_RESULT_TIMEOUT
}HNRW_RESULT_T;

class HNReqWaitAction
{
    private:
 
        std::mutex m;
        std::condition_variable cv;

        HNRW_RESULT_T resCode;

    public:
        HNReqWaitAction();
       ~HNReqWaitAction();

        // Called by requesting thread
        // to block until response is ready
        void wait();

        // Call by processing thread
        // when request has been completed.
        void complete( bool success );

        // Get the overall status of request
        // after completion, derived classes 
        // will provide thier own app specific
        // status
        HNRW_RESULT_T getStatus();
};

class HNReqWaitQueue
{
    private:
        // Mutex covering interactions
        std::mutex queueMutex;

        // The event fd
        int eventFD;

        // Running count of elements
        uint64_t postCnt;
        uint64_t releaseCnt;

        // The queues of elements
        std::list< HNReqWaitAction* > postQueue;

    public:
        HNReqWaitQueue();
       ~HNReqWaitQueue();

        bool init();

        int getEventFD();

        uint64_t getPostedCnt();
        void postAndWait( HNReqWaitAction *record );
        HNReqWaitAction* aquireRecord();

};

#endif // _HN_REQ_WAIT_QUEUE_H_
