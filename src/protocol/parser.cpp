#include "protocol/parser.hpp"

namespace mini_redis
{
  std::vector<command_t> parser::parse(const std::string &buffer)
  {
    // TODO: Implement RESP parsing logic
    // 1. Append new data to the internal buffer.
    // 2. Loop through the buffer to find complete RESP messages.
    //    - A message starts with '*' followed by the number of elements.
    //    - Each element is a bulk string, starting with '$' and its length.
    // 3. For each complete message, parse it into a command_t.
    // 4. Return a vector of all parsed commands.
    // 5. Keep any incomplete data in the internal buffer for the next call.
    
    buffer_ += buffer;
    std::vector<command_t> commands;
    // Dummy implementation
    if (!buffer_.empty()) {
        commands.push_back({"dummy_command", "key", "value"});
        buffer_.clear();
    }
    return commands;
  }
} // namespace mini_redis
