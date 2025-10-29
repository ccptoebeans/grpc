#ifndef CONNECTION_H
#define CONNECTION_H

// std
#include <functional>
#include <mutex>
#include <vector>

// grpc
#include <grpcpp/grpcpp.h>

// monolith_grpc
#include "channel.h"
#include "constants.h"
#include "jobs.h"

// prometheus
namespace prometheus_module {
class MetricRegistryInterface;
class CounterInterface;
class GaugeInterface;
}  // namespace prometheus_module

namespace monolith_grpc::client {

class Connection {
public:

  Connection();
  ~Connection();

  struct ConnectParams {
    std::string host;

    std::string root;
    std::string cert;
    std::string key;

    std::string server_name_override;
    int initial_reconnect_backoff_millis = -1;
    int max_reconnect_backoff_millis = -1;
  };

  void Connect(const ConnectParams& params);
  void Disconnect();

  [[nodiscard]] bool WaitForChannelState(
    const std::vector<grpc_connectivity_state>& states, int timeout_millis, const std::atomic<bool>& cancel
  );
  [[nodiscard]] grpc_connectivity_state channel_state();
  [[nodiscard]] bool ready() const;
  void UpdateChannelState();

  [[nodiscard]] std::shared_ptr<grpc::Channel> channel();
  // Don't need [[nodiscard]] for WithNonNullChannel.  It's frequently used for opportunistic updates.
  bool WithNonNullChannel(const std::function<void()>& lambda, BlockingMode blocking, const std::atomic<bool>& cancel);

  enum class ConnectionMessage {
    kConnected,
    kDisconnected
  };

  [[nodiscard]] std::shared_ptr<Channel<ConnectionMessage>> GetListener();

  void set_metric_registry(
    prometheus_module::MetricRegistryInterface* registry, const std::string& prefix,
    const std::map<std::string, std::string>& labels
  );

  [[nodiscard]] int id();

private:

  std::atomic<int> connection_id_;
  static std::mutex connection_id_mutex_;
  static std::atomic<int> next_connection_id_;
  [[nodiscard]] static int AssignConnectionId();

  // Ongoing connection attempts
  Jobs connection_attempts_;
  std::shared_ptr<std::atomic<bool>> cancel_connect_;
  std::mutex cancel_connect_lock_;

  std::shared_ptr<grpc::Channel> channel_;
  std::mutex channel_lock_;
  std::condition_variable channel_condition_;
  std::mutex disconnect_lock_;

  [[nodiscard]] grpc_connectivity_state GetChannelState(
    bool try_to_reconnect, BlockingMode blocking, const std::atomic<bool>& cancel
  );
  grpc_connectivity_state channel_state_;

  // Channel watcher
  std::atomic<bool> channel_replaced_;
  grpc::CompletionQueue channel_state_cq_;
  bool channel_state_notification_pending_;
  grpc_connectivity_state old_channel_state_;

  PublishingChannel<ConnectionMessage> connection_state_publisher_;

  // Metrics
  std::mutex metrics_lock_;

  struct Metrics {
    prometheus_module::MetricRegistryInterface* metric_registry_;
    std::string metric_prefix_;
    std::map<std::string, std::string> metric_labels_;

    prometheus_module::CounterInterface* connect_called_;
    prometheus_module::CounterInterface* connections_attempted_;
    prometheus_module::CounterInterface* connections_failed_;
    prometheus_module::CounterInterface* connections_cancelled_;
    prometheus_module::CounterInterface* connections_succeeded_;
    prometheus_module::CounterInterface* disconnections_due_to_failure_;
    prometheus_module::CounterInterface* disconnections_due_to_idleness_;
    prometheus_module::CounterInterface* disconnections_due_to_shutdown_;

    prometheus_module::GaugeInterface* channel_state_metric_;
    prometheus_module::GaugeInterface* channel_state_unknown_;
    prometheus_module::GaugeInterface* channel_state_idle_;
    prometheus_module::GaugeInterface* channel_state_connecting_;
    prometheus_module::GaugeInterface* channel_state_ready_;
    prometheus_module::GaugeInterface* channel_state_transient_failure_;
    prometheus_module::GaugeInterface* channel_state_shut_down_;

    Metrics()
      : metric_registry_(nullptr),

        connect_called_(nullptr),
        connections_attempted_(nullptr),
        connections_failed_(nullptr),
        connections_cancelled_(nullptr),
        connections_succeeded_(nullptr),
        disconnections_due_to_failure_(nullptr),
        disconnections_due_to_idleness_(nullptr),
        disconnections_due_to_shutdown_(nullptr),

        channel_state_metric_(nullptr),
        channel_state_unknown_(nullptr),
        channel_state_idle_(nullptr),
        channel_state_connecting_(nullptr),
        channel_state_ready_(nullptr),
        channel_state_transient_failure_(nullptr),
        channel_state_shut_down_(nullptr) {
    }
  };
  [[nodiscard]] Metrics CreateMetrics(
    prometheus_module::MetricRegistryInterface* registry, const std::string& prefix,
    const std::map<std::string, std::string>& labels
  );
  Metrics metrics_;

  void UpdateChannelStateMetrics(grpc_connectivity_state old_state, grpc_connectivity_state new_state);
};

}  // namespace monolith_grpc

#endif
