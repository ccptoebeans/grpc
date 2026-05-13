// Copyright © 2025 CCP ehf.
#ifndef STREAM_COUNTER_H
#define STREAM_COUNTER_H

#include <atomic>

class StreamCounter {
public:

  [[nodiscard]] static unsigned int AssignStreamId();

private:

  static std::atomic<unsigned int> stream_id_;
};

#endif
