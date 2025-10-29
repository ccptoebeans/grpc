#include "channel.h"

namespace monolith_grpc::client {

template<typename MessageType>
Channel<MessageType>::Channel()
  : closed_(false) {
}

template<typename MessageType>
void Channel<MessageType>::Write(const MessageType& message) {
  if (closed_) {
    return;
  }

  std::scoped_lock<std::mutex> lock(messages_lock_);
  messages_.push(message);
}

template<typename MessageType>
bool Channel<MessageType>::Read(MessageType& out) {
  if (closed_) {
    return false;
  }

  std::scoped_lock<std::mutex> lock(messages_lock_);

  if (messages_.empty()) {
    return false;
  }

  out = messages_.front();
  messages_.pop();

  return true;
}

// Permanently close the channel
template<typename MessageType>
void Channel<MessageType>::Close() {
  closed_ = true;
}

template<typename MessageType>
bool Channel<MessageType>::closed() const {
  return closed_;
}

template<typename MessageType>
size_t Channel<MessageType>::num_messages() const {
  if (closed_) {
    return 0;
  }

  std::scoped_lock<std::mutex> lock(messages_lock_);
  return messages_.size();
}

template<typename MessageType>
PublishingChannel<MessageType>::PublishingChannel()
  : closed_(false) {
}

// Write a message to all readers' queues.
template<typename MessageType>
void PublishingChannel<MessageType>::Write(const MessageType& message) {
  if (closed_) {
    return;
  }

  bool all_open = true;

  std::scoped_lock<std::mutex> lock(channels_lock_);

  // If any channels are closed, then we need to update our list of listeners
  for (auto& channel : channels_) {
    if (channel->closed()) {
      all_open = false;
      break;
    }
  }

  if (all_open) {
    // No need to update the listeners list, so just write to everyone normally.
    for (auto& channel : channels_) {
      channel->Write(message);
    }
  } else {
    // One or more listeners are closed, so rebuild our list as we write
    std::vector<std::shared_ptr<Channel<MessageType>>> channels;
    for (auto& channel : channels_) {
      channel->Write(message);
      if (!channel->closed()) {
        channels.emplace_back(std::move(channel));
      }
    }
    channels_ = channels;
  }
}

// Listener
template<typename MessageType>
std::shared_ptr<Channel<MessageType>> PublishingChannel<MessageType>::GetListener() {
  if (closed_) {
    return nullptr;
  }

  auto new_channel = std::make_shared<Channel<MessageType>>();

  std::scoped_lock<std::mutex> lock(channels_lock_);
  channels_.push_back(new_channel);
  return new_channel;
}

// Permanently close the channel for everyone.
template<typename MessageType>
void PublishingChannel<MessageType>::Close() {
  if (closed_) {
    return;
  }

  closed_ = true;

  std::scoped_lock<std::mutex> lock(channels_lock_);
  for (auto c : channels_) {
    c->Close();
  }

  channels_.clear();
}

template<typename MessageType>
bool PublishingChannel<MessageType>::closed() const {
  return closed_;
}

}  // namespace monolith_grpc
