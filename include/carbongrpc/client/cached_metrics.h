#ifndef CACHED_METRICS_H
#define CACHED_METRICS_H

// std
#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// prometheus
#include "counter_interface.h"
#include "gauge_interface.h"
#include "histogram_interface.h"
#include "metric_registry_interface.h"

namespace monolith_grpc::client {

class CachedCounter : public prometheus_module::CounterInterface {
public:

  // CachedCounter
  explicit CachedCounter(prometheus_module::CounterInterface* wrapped = nullptr);
  CachedCounter(const CachedCounter& other);             // Needed because atomics are not
                                                         // assignable or copyable
  CachedCounter& operator=(const CachedCounter& other);  // Needed because atomics are not assignable or copyable

  void Wrap(prometheus_module::CounterInterface* wrapped);
  void Commit();

  // CounterInterface
  void Increment() override;
  void Increment(double value) override;

  [[nodiscard]] CounterInterface* WithLabelValues(int num_values, const char* values[]) override;

private:

  prometheus_module::CounterInterface* wrapped_{};
  std::atomic<double> value_{};
  std::mutex lock_{};
};

class CachedGauge : public prometheus_module::GaugeInterface {
public:

  // CachedGauge
  explicit CachedGauge(prometheus_module::GaugeInterface* wrapped = nullptr);
  CachedGauge(const CachedGauge& other);             // Needed because atomics are not
                                                     // assignable or copyable
  CachedGauge& operator=(const CachedGauge& other);  // Needed because atomics are not assignable or copyable

  void Wrap(prometheus_module::GaugeInterface* wrapped);
  void Commit();

  // GaugeInterface
  void Increment() override;
  void Increment(double value) override;

  void Decrement() override;
  void Decrement(double value) override;

  void Set(double value) override;

  [[nodiscard]] GaugeInterface* WithLabelValues(int num_values, const char* values[]) override;

private:

  prometheus_module::GaugeInterface* wrapped_{};
  std::atomic<double> value_{};
};

class CachedHistogram : public prometheus_module::HistogramInterface {
public:

  // CachedHistogram
  explicit CachedHistogram(prometheus_module::HistogramInterface* wrapped = nullptr);
  CachedHistogram(const CachedHistogram& other);             // Needed because atomics are
                                                             // not assignable or copyable
  CachedHistogram& operator=(const CachedHistogram& other);  // Needed because mutexes are not assignable or copyable

  void Wrap(prometheus_module::HistogramInterface* wrapped);
  void Commit();

  // HistogramInterface
  void Observe(double value) override;

  [[nodiscard]] HistogramInterface* WithLabelValues(int num_values, const char* values[]) override;

private:

  prometheus_module::HistogramInterface* wrapped_{};
  std::vector<double> values_;
  std::mutex lock_;
};

class MetricsCreator {
public:

  MetricsCreator();

  void set_registry(prometheus_module::MetricRegistryInterface* registry);
  void set_prefix(const std::string& prefix);
  void set_labels(const std::map<std::string, std::string>& labels);

  void assign_label(const std::string& key, const std::string& value);
  void remove_label(const std::string& key);

  [[nodiscard]] CachedCounter MakeCounter(const std::string& name);
  [[nodiscard]] CachedGauge MakeGauge(const std::string& name);
  [[nodiscard]] CachedHistogram MakeHistogram(const std::string& name, const std::vector<double>& boundaries);

private:

  prometheus_module::MetricRegistryInterface* registry_;
  std::string prefix_;
  std::map<std::string, std::string> labels_;
};

}  // namespace monolith_grpc

#endif
