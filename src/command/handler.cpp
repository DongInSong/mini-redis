#include "command/handler.hpp"
#include <algorithm> // for std::transform
#include <cctype>   // for ::toupper

namespace mini_redis
{
  command_handler::command_handler(std::shared_ptr<store> store) : store_(store) {}

  std::string command_handler::execute_command(const command_t &cmd)
  {
    if (cmd.empty())
    {
      return "-ERR wrong number of arguments for 'empty' command\r\n";
    }

    std::string command_name = cmd[0];
    std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);

    if (command_name == "PING")
    {
      return handle_ping(cmd);
    }
    else if (command_name == "GET")
    {
      return handle_get(cmd);
    }
    else if (command_name == "SET")
    {
      return handle_set(cmd);
    }
    else if (command_name == "DEL")
    {
      return handle_del(cmd);
    }
    else if (command_name == "KEYS")
    {
      return handle_keys(cmd);
    }
    else
    {
      return "-ERR unknown command `" + cmd[0] + "`\r\n";
    }
  }

  std::string command_handler::handle_ping(const command_t &cmd)
  {
    if (cmd.size() > 2)
    {
      return "-ERR wrong number of arguments for 'ping' command\r\n";
    }
    if (cmd.size() == 2)
    {
      return "$" + std::to_string(cmd[1].length()) + "\r\n" + cmd[1] + "\r\n";
    }
    return "+PONG\r\n";
  }

  std::string command_handler::handle_get(const command_t &cmd)
  {
    if (cmd.size() != 2)
    {
      return "-ERR wrong number of arguments for 'get' command\r\n";
    }
    const std::string &key = cmd[1];
    std::optional<std::string> value = store_->get(key);
    if (value)
    {
      return "$" + std::to_string(value->length()) + "\r\n" + *value + "\r\n";
    }
    else
    {
      return "$-1\r\n";
    }
  }

  std::string command_handler::handle_set(const command_t &cmd)
  {
    if (cmd.size() != 3)
    {
      return "-ERR wrong number of arguments for 'set' command\r\n";
    }
    const std::string &key = cmd[1];
    const std::string &value = cmd[2];
    store_->set(key, value);
    return "+OK\r\n";
  }

  std::string command_handler::handle_del(const command_t &cmd)
  {
    if (cmd.size() != 2)
    {
      return "-ERR wrong number of arguments for 'del' command\r\n";
    }
    int deleted_count = store_->del(cmd[1]);
    return ":" + std::to_string(deleted_count) + "\r\n";
  }

  std::string command_handler::handle_keys(const command_t &cmd)
  {
    if (cmd.size() != 2)
    {
      return "-ERR wrong number of arguments for 'keys' command\r\n";
    }
    const std::string &pattern = cmd[1];
    std::vector<std::string> matched_keys = store_->keys(pattern);
    std::string response = "*" + std::to_string(matched_keys.size()) + "\r\n";
    for (const auto &key : matched_keys)
    {
      response += "$" + std::to_string(key.length()) + "\r\n" + key + "\r\n";
    }
    return response;
  }

} // namespace mini_redis
