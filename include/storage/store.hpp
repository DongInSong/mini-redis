#ifndef MINI_REDIS_STORE_HPP
#define MINI_REDIS_STORE_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <vector>

namespace mini_redis
{
  class store
  {
  public:
    /**
     * @brief Sets a key-value pair.
     * 
     * @param key The key.
     * @param value The value.
     */
    void set(const std::string &key, const std::string &value);

    /**
     * @brief Gets the value for a key.
     * 
     * @param key The key.
     * @return The value, or std::nullopt if the key does not exist.
     */
    std::optional<std::string> get(const std::string &key);

    /**
     * @brief Deletes a key.
     * 
     * @param key The key to delete.
     * @return 1 if the key was deleted, 0 otherwise.
     */
    int del(const std::string &key);

    /**
     * @brief Gets keys matching a pattern.
     * 
     * @param pattern The pattern to match.
     * @return A vector of matching keys.
     */
    std::vector<std::string> keys(const std::string &pattern = "*");

  private:
  // 해시 맵을 사용하여 키-값을 저장
    std::unordered_map<std::string, std::string> data_;
    std::mutex mutex_;
  };
} // namespace mini_redis

#endif // MINI_REDIS_STORE_HPP
