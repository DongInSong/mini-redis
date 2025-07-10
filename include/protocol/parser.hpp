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

  public:
    /**
     * @brief Serializes an error message into a RESP string.
     * 
     * @param message The error message.
     * @return The serialized error string.
     */
    static std::string serialize_error(const std::string &message);

    /**
     * @brief Serializes an integer into a RESP string.
     * 
     * @param value The integer value.
     * @return The serialized integer string.
     */
    static std::string serialize_integer(int value);
  };
} // namespace mini_redis

#endif // MINI_REDIS_PARSER_HPP
