#ifndef MINI_REDIS_PARSER_HPP
#define MINI_REDIS_PARSER_HPP

#include <string>
#include <vector>
#include <variant>

namespace mini_redis
{
  // Represents a single command parsed from the RESP protocol.
  using command_t = std::vector<std::string>;

  class parser
  {
  public:
    /**
     * @brief Parses a chunk of data from the client.
     * 
     * @param buffer The data to parse.
     * @return A vector of fully parsed commands.
     */
    std::vector<command_t> parse(const std::string &buffer);

  private:
    // Internal buffer for storing partial data.
    std::string buffer_;
  };
} // namespace mini_redis

#endif // MINI_REDIS_PARSER_HPP
