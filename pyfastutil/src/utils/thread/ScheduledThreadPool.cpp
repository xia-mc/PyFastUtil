//
// Created by xia__mc on 2024/11/12.
//

#include "ScheduledThreadPool.h"

static const unsigned int HARDWARE_THREADS = std::thread::hardware_concurrency();

__forceinline void ScheduledThreadPool::initThreads(const unsigned int count) {
    for (unsigned int i = 0; i < count; ++i) {
        threads.emplace_back([this]() { worker(); });
    }
}

__forceinline void ScheduledThreadPool::worker() {
    while (true) {
        std::unique_lock<std::mutex> curLock(mutex);
        tasksReady.wait(curLock, [this]() { return shutdown || !tasks.empty(); });

        if (shutdown) {
            return;
        }

        std::function<void()> task = tasks.front();
        tasks.pop();

        task();
    }
}

ScheduledThreadPool::ScheduledThreadPool() {
    initThreads(HARDWARE_THREADS);
}

ScheduledThreadPool::ScheduledThreadPool(const unsigned int threadCount) {
    initThreads(threadCount);
}

ScheduledThreadPool::~ScheduledThreadPool() {
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        shutdown = true;
    }

    tasksReady.notify_all();
    for (auto &thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
