#include "ThreadPool.h"

#include <iostream>

using namespace std;

//Job::Job(
//    const std::function<void()>&        aFunction )
//    : myIsDone(false)
//    , myFunction(aFunction)
//{
//}

//void
//Job::waitForCompletion()
//{
//    std::unique_lock<std::mutex> lk(myMutex);
//    //cout << "Waiting..." << endl;
//    myCV.wait(lk, [&]{return myIsDone;});
//    //cout << "Finished waiting" << endl;
//}

//void
//Job::execute()
//{
//	std::lock_guard<std::mutex> lk(myMutex);
//	myFunction();
//    myIsDone = true;
//    myCV.notify_all();
//}

ThreadPool::ThreadPool(
   int              aThreadCount)
{
    myWorking = true;
	myAddJobFlag = false;
    for (int i = 0; i < aThreadCount; i++)
    {
        myThreads.emplace_back(threadFunctionStatic, i);
//        myThreads.push_back( std::thread( &threadFunctionStatic, i) );
    }
}

ThreadPool::~ThreadPool()
{
//    cout << "In threadPool destructor" << endl;
    instance().stop();
}

shared_ptr<Job>
ThreadPool::addJob(
    const std::function<void()>&       aJobFunction)
{
    std::lock_guard<std::mutex> lk(myJobsMutex);
    myJobs.emplace_back(new Job(aJobFunction));
    myJobPresentCondition.notify_all();
    return myJobs.back();
}

ThreadPool&
ThreadPool::instance()
{
    static ThreadPool instance(8);

    return instance;
}

void
ThreadPool::stop()
{
	{
		std::lock_guard<std::mutex> lk(myJobsMutex);
		myWorking = false;
	}
	myJobPresentCondition.notify_all();
	for (auto& t: myThreads)
	{
        if (t.joinable())
		{
			t.join();
		}
    }
}

void
ThreadPool::threadFunctionStatic(
    int					aThreadId)
{
    instance().threadFunction(aThreadId);
}

void
ThreadPool::threadFunction(
    int                 aThreadId)
{
    while (true)
    {
        std::unique_lock<std::mutex> lk(myJobsMutex);
        myJobPresentCondition.wait(lk, [&]{return !myJobs.empty() || !myWorking;});
        if ( !myWorking )
        {
//            cout << "Stopping thread " << aThreadId << endl;
            return;
        }
        shared_ptr<Job> currentJob = myJobs.front();
        myJobs.pop_front();
        lk.unlock();
		myJobPresentCondition.notify_all();
        //cout << "Thread " << aThreadId << ", starting task" << endl;
        currentJob->execute();
        //cout << "Thread " << aThreadId << " finished task" << endl;
    }
}
