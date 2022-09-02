#ifndef XFUTURE_H
#define XFUTURE_H

#include <QObject>
#include <atomic>
#include <optional>
#include <mutex>

template<class T>
class XPromise {
    std::atomic_bool isCancelled;
    std::atomic_bool isStarted;

    std::mutex completionLock;
    std::optional<T> completedWith;
    std::function<void(T &)> onCompleteCB;

public:
    void start() {
        isStarted.store(true, std::memory_order_release);
    }

    void cancel() {
        isCancelled.store(true, std::memory_order_release);
    }

    bool started() {
        return isStarted.load(std::memory_order_acquire);
    }

    bool cancelled() {
        return isCancelled.load(std::memory_order_acquire);
    }

    void complete(T value) {
        std::lock_guard completionLock_(completionLock);
        completedWith = std::move(value);
        if (onCompleteCB) {
            onCompleteCB(value);
            onCompleteCB = std::function<void(T &)>();
        }
    }

    void onComplete(std::function<void(T &)> cb) {
        if (completedWith.has_value()) {
            cb(*completedWith);
        }
        else {
            std::lock_guard completionLock_(completionLock);
            if (completedWith.has_value()) {
                cb(*completedWith);
            }
            else {
                onCompleteCB = std::move(cb);
            }
        }
    }
};

#endif // XFUTURE_H
