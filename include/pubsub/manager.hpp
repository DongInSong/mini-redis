#ifndef MINI_REDIS_PUBSUB_MANAGER_HPP
#define MINI_REDIS_PUBSUB_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>

namespace mini_redis
{
  class session; // Forward declaration

  class pubsub_manager
  {
  public:
    void subscribe(const std::string &channel, std::shared_ptr<session> client);
    void unsubscribe(const std::string &channel, session* client);
    void unsubscribe_all(session* client);
    int publish(const std::string &channel, const std::string &message);

  private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::unordered_set<std::shared_ptr<session>>> subscriptions_;
  };
} // namespace mini_redis

#endif // MINI_REDIS_PUBSUB_MANAGER_HPP
