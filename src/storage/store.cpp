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

  int store::del(const std::string &key)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.erase(key);
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

    // key 벡터 생성
    // 만약 패턴이 "*"이면 모든 키를 반환
    // 그렇지 않으면 glob_match 함수를 사용하여 패턴에 맞는 키만 반환
    std::vector<std::string> keys;
    if (pattern == "*") {
        keys.reserve(data_.size());
        for (const auto &pair : data_) {
            keys.push_back(pair.first);
        }
    } else {
        for (const auto &pair : data_) {
            if (glob_match(pattern, pair.first)) {
                keys.push_back(pair.first);
            }
        }
    }
    return keys;
  }
} // namespace mini_redis
