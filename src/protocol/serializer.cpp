#include "protocol/serializer.hpp"

namespace mini_redis
{
  namespace serializer
  {
    std::string serialize_ok()
    {
      return "+OK\r\n";
    }

    std::string serialize_error(const std::string &message)
    {
      return "-" + message + "\r\n";
    }

    std::string serialize_null_bulk_string()
    {
      return "$-1\r\n"; // null bulk string
    }

    std::string serialize_bulk_string(const std::optional<std::string> &value)
    {
      if (value.has_value())
      {
        return "$" + std::to_string(value->size()) + "\r\n" + *value + "\r\n";
      }
      else
      {
        return "$-1\r\n"; // null bulk string
      }
    }

    std::string serialize_array(const std::vector<std::string> &values)
    {
      std::string result = "*" + std::to_string(values.size()) + "\r\n";
      for (const auto &v : values)
      {
        result += "$" + std::to_string(v.length()) + "\r\n" + v + "\r\n";
      }
      return result;
    }

    std::string serialize_integer(int value)
    {
      return ":" + std::to_string(value) + "\r\n";
    }
  } // namespace serializer
} // namespace mini_redis
