#include "Job.h"

Job::Job( const std::function<void()>& aFunction )
    : m_isDone(false)
    , m_function(aFunction)
{
}

void Job::waitForCompletion()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    //cout << "Waiting..." << endl;
    m_cv.wait(lk, [&]{return m_isDone;});
    //cout << "Finished waiting" << endl;
}

void Job::execute()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    m_function();
    m_isDone = true;
    m_cv.notify_one();
}
