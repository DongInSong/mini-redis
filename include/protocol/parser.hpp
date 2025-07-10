#ifndef MINI_REDIS_PARSER_HPP
#define MINI_REDIS_PARSER_HPP

#include <string>
#include <vector>
#include <variant>
#include <optional>

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
     * @brief Serializes a successful response into a RESP string.
     *
     * @return The serialized OK string.
     */
    static std::string serialize_ok();

    /**
     * @brief Serializes an error message into a RESP string.
     * 
     * @param message The error message.
     * @return The serialized error string.
     */
    static std::string serialize_error(const std::string &message);

    /**
     * @brief 
     * 
     * @return The serialized null bulk string.
     */
    static std::string serizlize_null_bulk_string();

    /**
     * @brief Serializes a bulk string into a RESP string.
     * 
     * @param value The string value, or std::nullopt for a null bulk string.
     * @return The serialized bulk string.
     */
    static std::string serialize_bulk_string(const std::optional<std::string> &value);

    /**
     * @brief Serializes an array of strings into a RESP string.
     * 
     * @param values The vector of strings to serialize.
     * @return The serialized array string.
     */
    static std::string serialize_array(const std::vector<std::string> &values);

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
