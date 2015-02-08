#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Job.h"

#include <memory>
#include <vector>
#include <list>
#include <condition_variable>
#include <thread>
#include <map>

class ThreadPool;

//class Job
//{
//public:
//                    Job(
//                        const std::function<void()>& aFunction);

//    void            waitForCompletion();
//    void            execute();
//private:
//    bool                        myIsDone;
//    std::function<void()>       myFunction;
//    std::condition_variable     myCV;
//    std::mutex                  myMutex;
//};

class ThreadPool
{
public:
                            ThreadPool(
                                int aThreadCount);

                            ~ThreadPool();

    std::shared_ptr<Job>    addJob(
                                const std::function<void()>&      aJobFunction);
    std::shared_ptr<Job>    addJob(
                                const std::function<void()>&    aJobFunction,
                                std::thread::id                 aThreadId );
    static ThreadPool&      instance();
    void                    stop();
private:
    static void             threadFunctionStatic(
                                int aThreadId);
    void                    threadFunction(
                                int aThreadId);

    std::vector<std::thread>            myThreads;
    std::map<std::thread::id, std::thread> m_threadMap;
    std::list<std::shared_ptr<Job> >    myJobs;
    std::condition_variable             myJobPresentCondition;
    std::mutex                          myJobsMutex;
    bool                                myWorking;

	bool								myAddJobFlag;
};

#endif // THREADPOOL_H
