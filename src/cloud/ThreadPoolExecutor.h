#pragma once

#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>
#include <functional>
#include <atomic>

#include "NonCopyable.h"

namespace jm::web::cloud
{
  class ThreadPoolExecutor : NonCopyable
  {
  public:

    ThreadPoolExecutor(int poolSize);

    ~ThreadPoolExecutor();

    void execute(std::function<void()>&& fn);

  private:
    void threadStart();

  private:
    bool isQueueEmpty() const;

    std::deque<std::function<void()>> _queue;
    mutable std::mutex _mutex;
    std::condition_variable _cv;
    std::condition_variable _cvJobComplete;
    std::vector<std::thread> _threads;
    int _initPoolSize;
    int _maxPoolSize;
  };

}
