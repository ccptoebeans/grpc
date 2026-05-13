// Copyright © 2025 CCP ehf.
#include "carbongrpc/client/jobs.h"
using namespace monolith_grpc::client;

// cppcheck-suppress unusedFunction
void Jobs::Run(const std::function<void()>& lambda) {
  Clean();
  std::future<void> job = std::async(std::launch::async, lambda);
  std::scoped_lock<std::mutex> lock(jobs_lock_);
  jobs_.push_back(std::move(job));
}

void Jobs::Wait() {
  std::scoped_lock<std::mutex> lock(jobs_lock_);
  for (auto&& job : jobs_) {
    job.get();
  }
  jobs_.clear();
}

void Jobs::Clean() {
  std::scoped_lock<std::mutex> lock(jobs_lock_);
  auto end_iter = std::remove_if(jobs_.begin(), jobs_.end(), [](auto&& job) {
    auto status = job.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
      return true;
    }
    return false;
  });
  jobs_.erase(end_iter, jobs_.end());
}
