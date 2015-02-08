#ifndef __JOB_H__
#define __JOB_H__

#include <condition_variable>
#include <functional>
#include <mutex>
#include <memory>

class Job
{
public:
                    Job( const std::function<void()>& aFunction );

    void            waitForCompletion();
    void            execute();
private:
    bool                        m_isDone;
    std::function<void()>       m_function;
    std::condition_variable     m_cv;
    std::mutex                  m_mutex;
};

typedef std::shared_ptr< Job > JobSp;

#endif // __JOB_H__
