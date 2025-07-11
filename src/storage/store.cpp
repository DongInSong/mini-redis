#include "storage/store.hpp"
#include <stdexcept>

namespace mini_redis
{
  // private helper function
  bool store::is_key_expired(const value_entry &entry)
  {
    if (entry.expiry.has_value())
    {
      if (std::chrono::steady_clock::now() > entry.expiry.value())
      {
        return true;
      }
    }
    return false;
  }

  void store::set(const std::string &key, const std::string &value)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    data_[key] = {RedisString(value), std::nullopt};
  }

  void store::setex(const std::string &key, int ttl_seconds, const std::string &value)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto expiry_time = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    data_[key] = {RedisString(value), expiry_time};
  }

  std::optional<std::string> store::get(const std::string &key)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it == data_.end())
    {
      return std::nullopt;
    }

    // Check if the key is expired
    if (is_key_expired(it->second))
    {
      data_.erase(it);
      return std::nullopt;
    }

    // Check if the value is a string
    if (auto val_ptr = std::get_if<RedisString>(&it->second.value))
    {
      return *val_ptr;
    }

    // The key exists but is not a string type
    return std::nullopt;
  }

  int store::del(const std::string &key)
  {
    return del(std::vector<std::string>{key});
  }

  // 멀티 키 삭제
  // 이 함수는 벡터로 전달된 여러 키를 삭제합니다.
  // 각 키에 대해 data_ 맵에서 해당 키를 찾아 삭제하고, 삭제된 키의 개수를 반환합니다.
  // 만약 키가 존재하지 않으면 삭제되지 않습니다.
  // 이 함수는 멀티 스레드 환경에서 안전하게 동작하도록 mutex를 사용하여 동기화합니다.
  int store::del(const std::vector<std::string> &keys)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    int deleted_count = 0;
    for (const auto &key : keys)
    {
      if (data_.erase(key))
      {
        deleted_count++;
      }
    }
    return deleted_count;
  }
  

  // Wild card matching
  bool glob_match(const std::string &pattern, const std::string &text) {
    size_t p_idx = 0, t_idx = 0;
    size_t p_len = pattern.length(), t_len = text.length();
    size_t star_idx = std::string::npos, t_match_idx = 0;

    while (t_idx < t_len) {

      // ? 또는 문자 일치
      if (p_idx < p_len && (pattern[p_idx] == '?' || pattern[p_idx] == text[t_idx])) {
        p_idx++;
        t_idx++;

        // * 일치 시, star_idx를 현재 위치로, t_match_idx를 현재 t_idx로 저장
      } else if (p_idx < p_len && pattern[p_idx] == '*') {
        star_idx = p_idx;
        t_match_idx = t_idx;
        p_idx++;

        // * 일치 후(이제 상관없이 끝까지 검색) 다음 일치되는 문자 찾기
      } else if (star_idx != std::string::npos) {
        p_idx = star_idx + 1;
        t_idx = ++t_match_idx;

        // 해당 없음
      } else {
        return false;
      }
    }

    // 연속된 '*' 처리
    // 패턴의 끝에서 '*'가 연속으로 있는 경우, 이를 무시하고 패턴이 끝났는지 확인
    while (p_idx < p_len && pattern[p_idx] == '*') {
      p_idx++;
    }

    return p_idx == p_len;
  }

  std::vector<std::string> store::keys(const std::string &pattern)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> matching_keys;
    std::vector<std::string> expired_keys;

    for (auto const& [key, val] : data_)
    {
      if (is_key_expired(val))
      {
        expired_keys.push_back(key);
        continue;
      }
      if (pattern == "*" || glob_match(pattern, key))
      {
        matching_keys.push_back(key);
      }
    }

    // Clean up expired keys
    for(const auto& key : expired_keys) {
        data_.erase(key);
    }

    return matching_keys;
  }

  bool store::exists(const std::string &key)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it == data_.end())
    {
      return false;
    }
    if (is_key_expired(it->second))
    {
      data_.erase(it);
      return false;
    }
    return true;
  }

  void store::expire(const std::string &key, int seconds)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it != data_.end())
    {
      if (seconds > 0)
      {
        it->second.expiry = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
      }
      else
      {
        it->second.expiry = std::nullopt;
      }
    }
  }

  long long store::ttl(const std::string &key)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it == data_.end())
    {
      return -2; // Key does not exist
    }

    if (is_key_expired(it->second))
    {
      data_.erase(it);
      return -2; // Key does not exist (as it just expired)
    }

    if (!it->second.expiry.has_value())
    {
      return -1; // Key exists but has no associated expire
    }

    auto now = std::chrono::steady_clock::now();
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(it->second.expiry.value() - now);
    return remaining.count();
  }

} // namespace mini_redis
