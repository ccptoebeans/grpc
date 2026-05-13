// Copyright © 2025 CCP ehf.
#include "carbongrpc/client/grpc_log.h"

#include <list>
#include <mutex>
#include <string>

namespace monolith_grpc::client {

GrpcLogEntry::GrpcLogEntry(std::string file, int line, gpr_log_severity severity, std::string message)
  : file(std::move(file)),
    line(line),
    severity(severity),
    message(std::move(message)) {
}

std::once_flag GrpcLog::init_flag_;

std::list<GrpcLogEntry> GrpcLog::log_;
std::mutex GrpcLog::lock_;

size_t GrpcLog::max_log_records_ = 10000;

void GrpcLog::Initialize() {
  std::call_once(GrpcLog::init_flag_, []() { gpr_set_log_function(GrpcLog::Log); });
}

// cppcheck-suppress unusedFunction
void GrpcLog::SetLogLevel(gpr_log_severity level) {
  Initialize();
  gpr_set_log_verbosity(level);
}

// cppcheck-suppress unusedFunction
std::list<GrpcLogEntry> GrpcLog::GetLogEntries() {
  Initialize();
  std::scoped_lock<std::mutex> lock(GrpcLog::lock_);

  std::list<GrpcLogEntry> result = std::move(GrpcLog::log_);
  GrpcLog::log_ = std::list<GrpcLogEntry>();

  return result;
}

void GrpcLog::Log(gpr_log_func_args* args) {
  std::scoped_lock<std::mutex> lock(GrpcLog::lock_);

  log_.emplace_back(GrpcLogEntry(args->file, args->line, args->severity, args->message));

  if (max_log_records_ > 0) {
    while (log_.size() > max_log_records_) {
      log_.pop_front();
    }
  }
}

}  // namespace monolith_grpc::client
