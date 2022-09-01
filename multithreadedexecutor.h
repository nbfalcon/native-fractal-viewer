#ifndef MULTITHREADEDEXECUTOR_H
#define MULTITHREADEDEXECUTOR_H

#include <functional>
#include <deque>
#include <QThread>
#include <QWaitCondition>


class MultithreadedExecutor
{
public:
    typedef void FN_executee(int threadNo, int ofTotalNThreads);

    MultithreadedExecutor();

    // Needs to be owned so we can do the work async
    void queueExecOnAll(std::function<FN_executee> executeMe);

private:
    class Worker : public QThread {
    public:
        Worker(MultithreadedExecutor &parent);
    };
    std::vector<Worker> workers;

    std::deque<std::function<FN_executee> > workQueue;
    QWaitCondition workIsAvailable;
    QMutex removingWorkLock;
    std::atomic_integer howManyMoreWorkersForWork;
};

#endif // MULTITHREADEDEXECUTOR_H
