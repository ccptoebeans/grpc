// Copyright © 2025 CCP ehf.
#pragma once

#include <atomic>
#include <mutex>
#include <queue>

namespace monolith_grpc::client {

/*
  Intended for one writer and one reader.
  Both sides should hold a shared_ptr<Channel<...>> and just drop it when
   done. The writer should close the channel before dropping the reference to
   let the reader know it's gone.
*/
template<typename MessageType>
class Channel {
public:

  Channel();

  // Write a message to the queue
  void Write(const MessageType& message);

  // Read a message from the queue into 'out'
  // Returns true if a message was received, or false if no message was ready
  [[nodiscard]] bool Read(MessageType& out);

  // Permanently close the channel
  void Close();

  bool closed() const;
  [[nodiscard]] size_t num_messages() const;

private:

  std::atomic<bool> closed_;
  mutable std::mutex messages_lock_;
  std::queue<MessageType> messages_;
};

/*
  Intended for one writer and multiple readers.
*/
template<typename MessageType>
class PublishingChannel {
public:

  PublishingChannel();

  // Write a message to all readers' queues.
  void Write(const MessageType& message);

  // Listener
  [[nodiscard]] std::shared_ptr<Channel<MessageType>> GetListener();

  // Permanently close the channel for everyone.
  void Close();
  [[nodiscard]] bool closed() const;

private:

  std::atomic<bool> closed_;
  std::mutex channels_lock_;
  std::vector<std::shared_ptr<Channel<MessageType>>> channels_;
};

}  // namespace monolith_grpc

#include "carbongrpc/client/channel.hpp"
