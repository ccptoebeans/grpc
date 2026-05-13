// Copyright © 2025 CCP ehf.
#ifndef JOBS_H
#define JOBS_H

#include <future>
#include <list>
#include <mutex>

namespace monolith_grpc::client {

class Jobs {
public:

  void Run(const std::function<void()>& lambda);
  void Wait();

private:

  void Clean();
  std::list<std::future<void>> jobs_;
  std::mutex jobs_lock_;
};

}  // namespace monolith_grpc

#endif
