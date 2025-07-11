#include "protocol/parser.hpp"
#include <stdexcept>
#include <iostream>

namespace mini_redis
{
  // RESP 형식 파싱
  /*
  예시. *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nhello\r\n -> SET, key, hello
    *3	  배열(Array), 총 3개의 항목
    $3	  첫 번째 항목은 길이 3의 문자열
    SET	  첫 번째 항목의 실제 값
    $3	  두 번째 항목도 길이 3의 문자열
    key	  두 번째 항목 값
    $5	  세 번째 항목은 길이 5의 문자열
    hello	세 번째 항목 값
  */
  std::vector<command_t> parser::parse(const std::string &data)
  {
    buffer_ += data;
    std::vector<command_t> commands;
    size_t pos = 0;

    while (pos < buffer_.size())
    {
      const auto start_pos = pos;
      try
      {
        // 배열 시작 기호 확인 *
        if (buffer_[pos] != '*')
        {
          throw std::runtime_error("Invalid RESP: command must start with '*'");
        }
        pos++;

        size_t crlf_pos = buffer_.find("\r\n", pos);
        if (crlf_pos == std::string::npos) 
          break; // \r\n이 없으면 파싱 중단 (명령어 PING, SET 등)
        int num_elements = std::stoi(buffer_.substr(pos, crlf_pos - pos));
        pos = crlf_pos + 2;

        if (num_elements < 0) {
            throw std::runtime_error("Invalid RESP: array length cannot be negative");
        }

        command_t command;
        command.reserve(num_elements);

        for (int i = 0; i < num_elements; ++i)
        {
          // 각 요소를 파싱
          if (buffer_[pos] != '$')
          {
            throw std::runtime_error("Invalid RESP: element must start with '$'");
          }
          pos++;

          crlf_pos = buffer_.find("\r\n", pos);
          if (crlf_pos == std::string::npos)
            break; // \r\n이 없으면 파싱 중단 (KEY)
          int bulk_len = std::stoi(buffer_.substr(pos, crlf_pos - pos));
          pos = crlf_pos + 2;

          if (bulk_len < 0) {
              throw std::runtime_error("Invalid RESP: bulk string length cannot be negative");
          }

          if (pos + bulk_len + 2 > buffer_.size())
            break; // 마지막 데이터 (VALUE)

          command.push_back(buffer_.substr(pos, bulk_len));
          pos += bulk_len + 2;
        }

        if (command.size() == static_cast<size_t>(num_elements)) {
            commands.push_back(std::move(command));
        } else {
            // 파싱이 중간에 멈췄으므로, 다음을 위해 대기합니다.
            pos = start_pos;
            break;
        }
      }
      catch (const std::exception &e)
      {
        // 파싱 오류가 발생하면, 해당 부분을 건너뛰고 다음을 시도할 수 있습니다.
        // 여기서는 간단하게 버퍼를 비우고 오류를 출력합니다.
        std::cerr << "Parsing error: " << e.what() << std::endl;
        buffer_.erase(0, pos); // 오류가 발생한 부분까지 버퍼를 비웁니다.
        pos = start_pos; // 위치 리셋
        break; 
      }
    }

    // 처리된 부분만큼 버퍼에서 제거합니다.
    if (pos > 0) {
        buffer_.erase(0, pos);
    }

    return commands;
  }

} // namespace mini_redis
