#include <iostream>
#include <thread>
#include <functional>
#include <unistd.h>

#include "ExecutorService.h"

using namespace std;

int main()
{
    ExecutorService executor( 8 );
    usleep( 100000 );

    thread::id mainThreadId = this_thread::get_id();
    cout << "Main thread id " << mainThreadId << endl;

    executor.addJob( [&](){
        std::hash<std::thread::id> idHash;
        std::thread::id threadId = this_thread::get_id();

        executor.addJob( [&](){

            JobSp j = executor.addJob( [&](){
                usleep( 100000 );
                cout << "Hi again from thread " << this_thread::get_id() << endl;
            }, threadId );

            j->waitForCompletion();

            j = executor.addJob( [&](){
                usleep( 100000 );
                cout << "Hi from MAIN thread " << this_thread::get_id() << endl;
            }, mainThreadId );

            j->waitForCompletion();

            usleep( 100000 );
            cout << "Hello from thread " << this_thread::get_id() << endl;
        });

        usleep( 100000 );
        cout << "Hello from working thread " << threadId << endl;
    } );

    executor.addJob( [&](){
        usleep( 1000000 );
        executor.addJob( [&](){ executor.stop(); }, mainThreadId );
    } );
    executor.workerFunction();

    usleep( 500000 );
    cout << "Hello World!" << endl;

    return 0;
}

