#ifndef _HN_SIG_SYNC_QUEUE_H_
#define _HN_SIG_SYNC_QUEUE_H_

#include <list>
#include <mutex>

template <class T>
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
        std::list< T > postQueue;
        std::list< T > releaseQueue;

    public:
        HNSigSyncQueue();
       ~HNSigSyncQueue();

        bool init();

        int getEventFD();

        uint64_t getPostedCnt();
        void postRecord( T record );
        T aquireRecord();

        uint64_t getReleasedCnt();
        void releaseRecord( T record );
        T freeRecord();
};

#endif
