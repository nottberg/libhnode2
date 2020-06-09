#ifndef _HN_EP_LOOP_H_
#define _HN_EP_LOOP_H_

#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <string>
#include <set>

typedef enum HNEPLoopResultEnum
{
    HNEP_RESULT_SUCCESS,
    HNEP_RESULT_FAILURE
} HNEP_RESULT_T;

class HNEPLoopCallbacks
{
    public:

        virtual void loopIteration() = 0;

        virtual void timeoutEvent() = 0;

        virtual void fdEvent( int sfd ) = 0;

        virtual void fdError( int sfd ) = 0; 
};

class HNEPLoop
{
    public:
        HNEPLoop();
       ~HNEPLoop();

        HNEP_RESULT_T setup( HNEPLoopCallbacks *parent );
        void stop();

        HNEP_RESULT_T run();

        HNEP_RESULT_T addFDToEPoll( int sfd );
        HNEP_RESULT_T removeFDFromEPoll( int sfd );

    private:
        bool m_quit;
        uint m_timeoutMS;

        std::set< int > m_clientSet;

        HNEPLoopCallbacks *m_parent;

        int m_epollFD;
        struct epoll_event *m_events;

};

#endif // _HN_EP_LOOP_H_
