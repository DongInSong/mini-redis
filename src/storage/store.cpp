#include "storage/store.hpp"

namespace mini_redis
{
  void store::set(const std::string &key, const std::string &value)
  {
    // TODO: Implement set logic
    // 1. Lock the mutex.
    // 2. Insert or update the key-value pair in the map.
    // 3. Unlock the mutex.
    std::lock_guard<std::mutex> lock(mutex_);
    data_[key] = value;
  }

  std::optional<std::string> store::get(const std::string &key)
  {
    // TODO: Implement get logic
    // 1. Lock the mutex.
    // 2. Find the key in the map.
    // 3. If found, return the value. Otherwise, return std::nullopt.
    // 4. Unlock the mutex.
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it != data_.end())
    {
      return it->second;
    }
    return std::nullopt;
  }

  void store::del(const std::string &key)
  {
    // TODO: Implement del logic
    // 1. Lock the mutex.
    // 2. Erase the key from the map.
    // 3. Unlock the mutex.
    std::lock_guard<std::mutex> lock(mutex_);
    data_.erase(key);
  }
} // namespace mini_redis
