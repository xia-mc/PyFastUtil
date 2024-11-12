//
// Created by xia__mc on 2024/11/12.
//

#ifndef PYFASTUTIL_SCHEDULEDTHREADPOOL_H
#define PYFASTUTIL_SCHEDULEDTHREADPOOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <queue>

class ScheduledThreadPool {
public:
    ScheduledThreadPool();

    explicit ScheduledThreadPool(unsigned int threadCount);

    ~ScheduledThreadPool();

    template<typename T>
    inline std::future<T> submit(const std::function<T()> &func) {
        auto task = std::packaged_task<T()>(func);
        std::future<T> future = task.get_future();

        std::lock_guard<std::mutex> lock(mutex);
        std::function<void()> finalTask = [&task](){task();};
        tasks.emplace(finalTask);

        tasksReady.notify_one();

        return future;
    }

    inline std::future<void> submit(const std::function<void()> &func) {
        auto task = std::packaged_task<void()>(func);
        std::future<void> future = task.get_future();

        std::lock_guard<std::mutex> lock(mutex);
        std::function<void()> finalTask = [&task](){task();};
        tasks.emplace(finalTask);

        tasksReady.notify_one();

        return future;
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable tasksReady;
    std::atomic<bool> shutdown = std::atomic_bool(false);

    __forceinline void initThreads(unsigned int count);

    __forceinline void worker();
};

#endif //PYFASTUTIL_SCHEDULEDTHREADPOOL_H
