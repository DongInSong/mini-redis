#ifndef MINI_REDIS_SERIALIZER_HPP
#define MINI_REDIS_SERIALIZER_HPP

#include <string>
#include <vector>
#include <optional>

namespace mini_redis
{
  namespace serializer
  {
    /**
     * @brief Serializes a successful response into a RESP string.
     *
     * @return The serialized OK string.
     */
    std::string serialize_ok();

    /**
     * @brief Serializes an error message into a RESP string.
     * 
     * @param message The error message.
     * @return The serialized error string.
     */
    std::string serialize_error(const std::string &message);

    /**
     * @brief Serializes a null bulk string.
     * 
     * @return The serialized null bulk string.
     */
    std::string serialize_null_bulk_string();

    /**
     * @brief Serializes a bulk string into a RESP string.
     * 
     * @param value The string value, or std::nullopt for a null bulk string.
     * @return The serialized bulk string.
     */
    std::string serialize_bulk_string(const std::optional<std::string> &value);

    /**
     * @brief Serializes an array of strings into a RESP string.
     * 
     * @param values The vector of strings to serialize.
     * @return The serialized array string.
     */
    std::string serialize_array(const std::vector<std::string> &values);

    /**
     * @brief Serializes an integer into a RESP string.
     * 
     * @param value The integer value.
     * @return The serialized integer string.
     */
    std::string serialize_integer(int value);
  } // namespace serializer
} // namespace mini_redis

#endif // MINI_REDIS_SERIALIZER_HPP
