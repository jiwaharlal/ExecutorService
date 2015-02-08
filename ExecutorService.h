#ifndef __EXECUTOR_SERVICE_H__
#define __EXECUTOR_SERVICE_H__

#include "Job.h"

#include <queue>
#include <memory>
#include <map>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <set>
#include <vector>

class ExecutorService
{
public:
    ExecutorService(int threadCount);
    ~ExecutorService();

//    static ExecutorService& instance();

    typedef std::queue< std::shared_ptr< Job > > JobQueue;

    std::shared_ptr< Job > addJob( const std::function< void() >& func );
    std::shared_ptr< Job > addJob( const std::function< void() >& func, std::thread::id threadId );

    struct JobQueueStruct
    {
        JobQueue queue;
        std::mutex mutex;
        std::condition_variable condition;
    };

    void workerFunction();
    void stop();
private:
    void dispatcherFunction();

    bool m_stop;

    typedef std::vector< std::thread > ThreadVector;
    ThreadVector m_threads;
    std::thread m_dispatcher;

    typedef std::map< std::thread::id, JobQueueStruct* > JobQueueMap;
    JobQueueMap m_threadQueuesMap;
    std::mutex m_threadJobsMutex;

    typedef std::set< std::thread::id > ThreadIdSet;
    ThreadIdSet m_joblessThreads;
    std::mutex m_joblessMutex;
    std::condition_variable m_joblessCondition;

    JobQueue m_commonJobQueue;
    std::mutex m_commonQueueMutex;
    std::condition_variable m_commonQueueCondition;

};

#endif // __EXECUTOR_SERVICE_H__
