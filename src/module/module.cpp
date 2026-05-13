// Copyright © 2025 CCP ehf.
#include "carbongrpc/module/module.h"

#include <functional>
#include <list>
#include <system_error>
#include <thread>

#include "Python.h"

// Force init_message to be linked
PyMODINIT_FUNC PyInit__message();
void init_message_link() {
  PyInit__message();
}

namespace monolith_grpc::module {

std::atomic<int> num_active_publishers;

std::list<std::function<void()>>& get_finalizers() {
  static std::list<std::function<void()>> finalize_functions;
  return finalize_functions;
}

void RegisterPublisherFinalizeFunction(const std::function<void()>& func) {
  auto& finalize_functions = get_finalizers();
  finalize_functions.push_back(func);
}

std::list<std::thread> shutdown_threads;
void AddShutdownThread(std::thread t) {
  shutdown_threads.push_back(std::move(t));
}

bool PublisherFinalize() {
  if (num_active_publishers > 0) {
    return false;
  }

  std::atomic<bool> worker_shutdown_complete(false);  ///< atomic just for memory barrier, not sensitive to sequence
  std::thread worker_shutdown([&] {
    for (auto&& t : shutdown_threads) {
      if (t.joinable()) {
        try {
          t.join();
        } catch (std::system_error&) {
          // If the thread completes between the joinable() and join() calls, it
          // could throw this exception. It's harmless, just signifying that the
          // thread has exited, so we don't need to do anything.
        }
      }
    }

    worker_shutdown_complete.store(true);
  });

  const auto& finalize_functions = get_finalizers();
  for (const auto& func : finalize_functions) {
    func();
  }

  auto finalizer_start = std::chrono::steady_clock::now();
  while (!worker_shutdown_complete.load()) {
    auto finalizer_duration = std::chrono::steady_clock::now() - finalizer_start;
    if (finalizer_duration > std::chrono::seconds(10)) {
      printf(
        "One or more grpc client threads failed to shut down before the "
        "deadline\n"
      );
      worker_shutdown.detach();
      break;
    }
  }

  if (worker_shutdown.joinable() && worker_shutdown_complete.load()) {
    worker_shutdown.join();
  }

  return true;
}

}  // namespace monolith_grpc::module
