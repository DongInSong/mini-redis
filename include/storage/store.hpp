#ifndef MINI_REDIS_STORE_HPP
#define MINI_REDIS_STORE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <map>
#include <mutex>
#include <optional>
#include <variant>
#include <chrono>

namespace mini_redis
{
  // Redis 데이터 타입 정의
  using RedisString = std::string;
  using RedisList = std::list<std::string>;
  using RedisHash = std::unordered_map<std::string, std::string>;
  using RedisSet = std::unordered_set<std::string>;
  using RedisSortedSet = std::map<double, std::string>; // score -> member

  // std::variant를 사용하여 다양한 데이터 타입을 저장
  using RedisValue = std::variant<
      RedisString,
      RedisList,
      RedisHash,
      RedisSet,
      RedisSortedSet>;

  // 값과 만료 시간을 저장하는 구조체
  struct value_entry
  {
    RedisValue value;
    std::optional<std::chrono::steady_clock::time_point> expiry;
  };

  class store
  {
  public:
    // String commands
    void set(const std::string &key, const std::string &value);
    void setex(const std::string &key, int ttl_seconds, const std::string &value);
    std::optional<std::string> get(const std::string &key);
    
    // Generic commands
    int del(const std::string &key);
    int del(const std::vector<std::string> &keys);
    std::vector<std::string> keys(const std::string &pattern = "*");
    bool exists(const std::string &key);
    void expire(const std::string &key, int seconds);
    long long ttl(const std::string &key);

  private:
    bool is_key_expired(const value_entry &entry);

    std::unordered_map<std::string, value_entry> data_;
    std::mutex mutex_;
  };
} // namespace mini_redis

#endif // MINI_REDIS_STORE_HPP
