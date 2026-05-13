// Copyright © 2025 CCP ehf.
#ifndef GATEWAY_MODULE_H
#define GATEWAY_MODULE_H

#include <atomic>
#include <thread>

namespace monolith_grpc::module {

void AddShutdownThread(std::thread t);
extern std::atomic<int> num_active_publishers;

}  // namespace monolith_grpc::module

#endif
