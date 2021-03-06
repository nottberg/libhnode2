#ifndef _HN_SIG_SYNC_QUEUE_H_
#define _HN_SIG_SYNC_QUEUE_H_

#include <list>
#include <mutex>

class HNSigSyncQueue
{
    private:
        // Mutex covering interactions
        std::mutex lm;

        // The event fd
        int eventFD;

        // Running count of elements
        uint64_t postCnt;
        uint64_t releaseCnt;

        // The queues of elements
        std::list< void* > postQueue;
        std::list< void* > releaseQueue;

    public:
        HNSigSyncQueue();
       ~HNSigSyncQueue();

        bool init();

        int getEventFD();

        uint64_t getPostedCnt();
        void postRecord( void* record );
        void* aquireRecord();

        uint64_t getReleasedCnt();
        void releaseRecord( void* record );
        void* freeRecord();
};

#endif
