#include "ThreadPoolExecutor.h"
#include <cassert>
#include <iostream>

using namespace jm::web::cloud;

ThreadPoolExecutor::ThreadPoolExecutor(int poolSize)
  : _maxPoolSize(poolSize)
{
  _threads.reserve(_maxPoolSize);
  for (int i = 0; i < _maxPoolSize; ++i) {
    std::thread t([this] {threadStart(); });
    _threads.emplace_back(std::move(t));
  }
}

ThreadPoolExecutor::~ThreadPoolExecutor()
{
  std::vector<std::thread> threads;

  while (!isQueueEmpty())
  {
    std::unique_lock<std::mutex> lk(_mutex);
    _cvJobComplete.wait(lk);
  }

  {
    std::unique_lock<std::mutex> lk(_mutex);
    threads.swap(_threads);
    _cv.notify_all();
  }

  for (auto& t : threads) {
    t.join();
  } 
}

bool ThreadPoolExecutor::isQueueEmpty() const
{
  std::unique_lock<std::mutex> lk(_mutex);
  return _queue.empty();
}

void ThreadPoolExecutor::execute(std::function<void()>&& fn)
{
  std::unique_lock<std::mutex> lk(_mutex);
  _queue.emplace_back(std::move(fn));
  _cv.notify_one();
}

void ThreadPoolExecutor::threadStart() {
  for (;;) {
    std::function<void()> fn;
    {
      std::unique_lock<std::mutex> lk(_mutex);
      if (_threads.empty())
        break;
      if (_queue.empty()) {
        _cv.wait(lk);
      }
      fn = std::move(_queue.front());
      _queue.pop_front();
    }
    fn();
    _cvJobComplete.notify_one();
  }
}
