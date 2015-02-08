#include "ExecutorService.h"

#include <cassert>

ExecutorService::ExecutorService( int threadCount )
    : m_stop( false )
{
    m_dispatcher = std::thread( std::bind( &ExecutorService::dispatcherFunction, this ) );

    for ( int i = 0; i < threadCount; i++ )
    {
        m_threads.emplace_back( std::bind( &ExecutorService::workerFunction, this ) );
    }
}

ExecutorService::~ExecutorService()
{
    if ( ! m_stop )
    {
        stop();
    }
}

//ExecutorService &ExecutorService::instance()
//{
//    static ExecutorService instance( 8 );
//    return instance;
//}

std::shared_ptr<Job> ExecutorService::addJob( const std::function<void ()> &func )
{
    JobSp newJob( new Job( func ) );

    {
        std::unique_lock< std::mutex > lk( m_commonQueueMutex );
        m_commonJobQueue.push( newJob );
    }
    m_commonQueueCondition.notify_one();

    return newJob;
}

std::shared_ptr<Job> ExecutorService::addJob( const std::function<void()> &func, std::thread::id threadId )
{
    JobSp newJob( new Job( func ) );

    JobQueueStruct* jobQueueStruct;
    {
        std::lock_guard< std::mutex > lk( m_threadJobsMutex );
        JobQueueMap::iterator it = m_threadQueuesMap.find( threadId );

        assert( it != m_threadQueuesMap.end() && "Thread jobs queue not found" );

        jobQueueStruct = it->second;
    }

    {
        std::unique_lock< std::mutex > lk( jobQueueStruct->mutex );
        jobQueueStruct->queue.push( newJob );
    }

    {
        std::lock_guard< std::mutex > lk( m_joblessMutex );
        ThreadIdSet::iterator it = m_joblessThreads.find( threadId );
        if ( it != m_joblessThreads.end() )
        {
            m_joblessThreads.erase( it );
            jobQueueStruct->condition.notify_one();
        }
    }

    return newJob;
}

void ExecutorService::dispatcherFunction()
{
    while ( true )
    {
        std::unique_lock< std::mutex > jobsLock( m_commonQueueMutex );
        m_commonQueueCondition.wait( jobsLock, [&]{ return ! m_commonJobQueue.empty() || m_stop; } );
        if ( m_stop )
        {
            break;
        }
        JobSp job = m_commonJobQueue.front();
        m_commonJobQueue.pop();
        jobsLock.unlock();

        std::unique_lock< std::mutex > joblessLock( m_joblessMutex );
        m_joblessCondition.wait( joblessLock, [&]{ return ! m_joblessThreads.empty() || m_stop; } );
        if ( m_stop )
        {
            break;
        }
        std::thread::id joblessThreadId = *( m_joblessThreads.begin() );
        m_joblessThreads.erase( m_joblessThreads.begin() );
        joblessLock.unlock();

        JobQueueStruct* queueStruct;
        {
            std::lock_guard< std::mutex > lk( m_threadJobsMutex );
            queueStruct = m_threadQueuesMap[ joblessThreadId ];
        }

        std::unique_lock< std::mutex > threadJobsLock( queueStruct->mutex );
        queueStruct->queue.push( job );
        threadJobsLock.unlock();
        queueStruct->condition.notify_one();
    }
}

void ExecutorService::workerFunction()
{
    std::thread::id threadId = std::this_thread::get_id();
    JobQueueStruct jobQueueStruct;

    {
        std::unique_lock< std::mutex > lk( m_threadJobsMutex );
        m_threadQueuesMap[ threadId ] = &jobQueueStruct;
    }

    while ( true ) {
        if ( jobQueueStruct.queue.empty() )
        {
            {
                std::lock_guard< std::mutex > lk( m_joblessMutex );
                m_joblessThreads.insert( threadId );
            }
            m_joblessCondition.notify_one();
        }

        std::unique_lock< std::mutex > lk( jobQueueStruct.mutex );
        jobQueueStruct.condition.wait( lk, [&]{ return ! jobQueueStruct.queue.empty() || m_stop; } );
        if ( m_stop )
        {
            break;
        }
        JobSp job = jobQueueStruct.queue.front();
        jobQueueStruct.queue.pop();
        lk.unlock();

        job->execute();
    }
}

void ExecutorService::stop()
{
    m_stop = true;
    m_commonQueueCondition.notify_one();
    m_joblessCondition.notify_one();
    m_dispatcher.join();
    for ( auto it = m_threadQueuesMap.begin(); it != m_threadQueuesMap.end(); ++it )
    {
        JobQueueStruct* queueStruct = it->second;
        queueStruct->condition.notify_one();
    }
    for ( auto it = m_threads.begin(); it != m_threads.end(); ++it )
    {
        it->join();
    }
}
