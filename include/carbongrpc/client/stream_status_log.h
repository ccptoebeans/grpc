#ifndef STREAM_STATUS_LOG_H
#define STREAM_STATUS_LOG_H

#include <grpcpp/grpcpp.h>

#include <list>
#include <vector>

namespace monolith_grpc::client {
struct StreamStatus {
  unsigned int stream_id;
  ::grpc::Status status;

  StreamStatus(unsigned int stream_id, ::grpc::Status  status);
};

class StreamStatusLog {
public:

  StreamStatusLog();

  void RecordStatus(const StreamStatus& status);
  void RecordStatus(unsigned int stream_id, const ::grpc::Status& status);
  [[nodiscard]] std::list<StreamStatus> GetStatusLog();

private:

  std::list<StreamStatus> log_;
  std::mutex lock_;

  size_t max_log_records_;
};
}  // namespace monolith_grpc

#endif
