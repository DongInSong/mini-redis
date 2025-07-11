#include "pubsub/manager.hpp"
#include "network/session.hpp"
#include "protocol/serializer.hpp"

namespace mini_redis
{
  void pubsub_manager::subscribe(const std::string &channel, std::shared_ptr<session> client)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_[channel].insert(client);
  }

  void pubsub_manager::unsubscribe(const std::string &channel, session* client)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscriptions_.find(channel);
    if (it == subscriptions_.end())
      return;

    for (auto sub_it = it->second.begin(); sub_it != it->second.end(); ++sub_it) {
      if (sub_it->get() == client) {
        it->second.erase(sub_it);
        break;
      }
    }

    if (it->second.empty())
    {
      subscriptions_.erase(it);
    }
  }

  void pubsub_manager::unsubscribe_all(session* client)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = subscriptions_.begin(); it != subscriptions_.end();)
    {
      for (auto sub_it = it->second.begin(); sub_it != it->second.end();) {
        if (sub_it->get() == client) {
          sub_it = it->second.erase(sub_it);
        } else {
          ++sub_it;
        }
      }

      if (it->second.empty())
      {
        it = subscriptions_.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  int pubsub_manager::publish(const std::string &channel, const std::string &message)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = subscriptions_.find(channel);
    if (it == subscriptions_.end())
      return 0;

    std::vector<std::string> response_parts = {"message", channel, message};
    std::string serialized_message = serializer::serialize_array(response_parts);

    for (const auto &client : it->second)
    {
      client->deliver(serialized_message);
    }

    return static_cast<int>(it->second.size());
  }
} // namespace mini_redis
