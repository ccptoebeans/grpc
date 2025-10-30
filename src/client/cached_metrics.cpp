#include "carbongrpc/client/cached_metrics.h"
using namespace monolith_grpc::client;

// CachedCounter
CachedCounter::CachedCounter(prometheus_module::CounterInterface* wrapped)
  : wrapped_(wrapped),
    value_(0.0) {
}

CachedCounter::CachedCounter(const CachedCounter& other) {
  *this = other;
}

CachedCounter& CachedCounter::operator=(const CachedCounter& other) {
  if(&other == this) {
    return *this;
  }

  wrapped_ = other.wrapped_;
  value_.store(other.value_.load());

  return *this;
}

// cppcheck-suppress unusedFunction
void CachedCounter::Wrap(prometheus_module::CounterInterface* wrapped) {
  wrapped_ = wrapped;
}

// cppcheck-suppress unusedFunction
void CachedCounter::Commit() {
  std::scoped_lock<std::mutex> lock(lock_);
  if (wrapped_ && value_ > 0.0) {
    wrapped_->Increment(value_);
    value_ = 0.0;
  }
}

// CounterInterface
void CachedCounter::Increment() {
  std::scoped_lock<std::mutex> lock(lock_);
  value_ = value_ + 1.0;
}

void CachedCounter::Increment(double value) {
  std::scoped_lock<std::mutex> lock(lock_);
  value_ = value_ + value;
}

prometheus_module::CounterInterface* CachedCounter::WithLabelValues(int num_values, const char* values[]) {
  if (wrapped_) {
    wrapped_ = wrapped_->WithLabelValues(num_values, values);
  }

  return this;
}

// CachedGauge
CachedGauge::CachedGauge(prometheus_module::GaugeInterface* wrapped)
  : wrapped_(wrapped),
    value_(0.0) {
}

CachedGauge::CachedGauge(const CachedGauge& other) {
  *this = other;
}

CachedGauge& CachedGauge::operator=(const CachedGauge& other) {
  if(&other == this) {
    return *this;
  }

  wrapped_ = other.wrapped_;
  value_.store(other.value_.load());

  return *this;
}

void CachedGauge::Wrap(prometheus_module::GaugeInterface* wrapped) {
  wrapped_ = wrapped;
}

void CachedGauge::Commit() {
  if (wrapped_) {
    wrapped_->Set(value_);
  }
}

// GaugeInterface
void CachedGauge::Increment() {
  value_ = value_ + 1.0;
}

void CachedGauge::Increment(double value) {
  value_ = value_ + value;
}

// cppcheck-suppress unusedFunction
void CachedGauge::Decrement() {
  value_ = value_ - 1.0;
}

void CachedGauge::Decrement(double value) {
  value_ = value_ - value;
}

void CachedGauge::Set(double value) {
  value_ = value;
}

prometheus_module::GaugeInterface* CachedGauge::WithLabelValues(int num_values, const char* values[]) {
  if (wrapped_) {
    wrapped_ = wrapped_->WithLabelValues(num_values, values);
  }

  return this;
}

// CachedHistogram
CachedHistogram::CachedHistogram(prometheus_module::HistogramInterface* wrapped)
  : wrapped_(wrapped) {
}

CachedHistogram::CachedHistogram(const CachedHistogram& other) {
  *this = other;
}

CachedHistogram& CachedHistogram::operator=(const CachedHistogram& other) {
  if(&other == this) {
    return *this;
  }

  std::scoped_lock<std::mutex> lock(lock_);

  wrapped_ = other.wrapped_;
  values_ = other.values_;

  return *this;
}

void CachedHistogram::Wrap(prometheus_module::HistogramInterface* wrapped) {
  wrapped_ = wrapped;
}

void CachedHistogram::Commit() {
  std::scoped_lock<std::mutex> lock(lock_);

  if (wrapped_) {
    for (auto value : values_) {
      wrapped_->Observe(value);
    }
  }

  values_.clear();
}

// HistogramInterface
void CachedHistogram::Observe(double value) {
  std::scoped_lock<std::mutex> lock(lock_);
  values_.push_back(value);
}

prometheus_module::HistogramInterface* CachedHistogram::WithLabelValues(int num_values, const char* values[]) {
  if (wrapped_) {
    wrapped_ = wrapped_->WithLabelValues(num_values, values);
  }

  return this;
}

// MetricsCreator

MetricsCreator::MetricsCreator()
  : registry_(nullptr) {
}

// cppcheck-suppress unusedFunction
void MetricsCreator::set_registry(prometheus_module::MetricRegistryInterface* registry) {
  registry_ = registry;
}

// cppcheck-suppress unusedFunction
void MetricsCreator::set_prefix(const std::string& prefix) {
  prefix_ = prefix;
}

// cppcheck-suppress unusedFunction
void MetricsCreator::set_labels(const std::map<std::string, std::string>& labels) {
  labels_ = labels;
}

// cppcheck-suppress unusedFunction
void MetricsCreator::assign_label(const std::string& key, const std::string& value) {
  labels_[key] = value;
}

// cppcheck-suppress unusedFunction
void MetricsCreator::remove_label(const std::string& key) {
  labels_.erase(key);
}

CachedCounter MetricsCreator::MakeCounter(const std::string& name) {
  if (registry_ == nullptr) {
    return CachedCounter(nullptr);
  }

  std::string prefixed_name = prefix_ + name;

  std::vector<const char*> keys;
  std::vector<const char*> values;
  for (auto& label : labels_) {
    keys.push_back(label.first.c_str());
    values.push_back(label.second.c_str());
  }

  return CachedCounter(registry_->MakeCounter(prefixed_name.c_str(), (int)keys.size(), keys.data())
                         ->WithLabelValues((int)values.size(), values.data()));
}

CachedGauge MetricsCreator::MakeGauge(const std::string& name) {
  if (registry_ == nullptr) {
    return CachedGauge(nullptr);
  }

  std::string prefixed_name = prefix_ + name;

  std::vector<const char*> keys;
  std::vector<const char*> values;
  for (auto& label : labels_) {
    keys.push_back(label.first.c_str());
    values.push_back(label.second.c_str());
  }

  return CachedGauge(registry_->MakeGauge(prefixed_name.c_str(), (int)keys.size(), keys.data())
                       ->WithLabelValues((int)values.size(), values.data()));
}

CachedHistogram MetricsCreator::MakeHistogram(const std::string& name, const std::vector<double>& boundaries) {
  if (registry_ == nullptr) {
    return CachedHistogram(nullptr);
  }

  std::string prefixed_name = prefix_ + name;

  std::vector<const char*> keys;
  std::vector<const char*> values;
  for (auto& label : labels_) {
    keys.push_back(label.first.c_str());
    values.push_back(label.second.c_str());
  }

  return CachedHistogram(registry_
                           ->MakeHistogram(
                             prefixed_name.c_str(), (int)keys.size(), keys.data(), (int)boundaries.size(),
                             const_cast<double*>(boundaries.data())
                           )
                           ->WithLabelValues((int)values.size(), values.data()));
}
