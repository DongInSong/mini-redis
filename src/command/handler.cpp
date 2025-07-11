#include "command/handler.hpp"
#include "protocol/serializer.hpp"
#include <algorithm> 
#include <cctype> 

namespace mini_redis
{
  command_handler::command_handler(std::shared_ptr<store> store) : store_(store) {}

  std::string command_handler::execute_command(const command_t &cmd)
  {
    if (cmd.empty())
    {
      // return "-ERR wrong number of arguments for 'empty' command\r\n";
      return serializer::serialize_error("ERR wrong number of arguments for 'empty' command");
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
    else if (command_name == "SETEX")
    {
      return handle_setex(cmd);
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
      return serializer::serialize_error("ERR unknown command `" + cmd[0] + "`");
    }
  }

  std::string command_handler::handle_ping(const command_t &cmd)
  {
    if (cmd.size() > 2)
    {
      return serializer::serialize_error("ERR wrong number of arguments for 'ping' command");
    }
    if (cmd.size() == 2)
    {
      return serializer::serialize_bulk_string(cmd[1]);
    }
    return serializer::serialize_ok();
  }

  std::string command_handler::handle_get(const command_t &cmd)
  {
    if (cmd.size() != 2)
    {
      return serializer::serialize_error("ERR wrong number of arguments for 'get' command");
    }
    const std::string &key = cmd[1];
    std::optional<std::string> value = store_->get(key);
    if (value)
    {
      return serializer::serialize_bulk_string(value);
    }
    else
    {
      return serializer::serialize_null_bulk_string();
    }
  }

  std::string command_handler::handle_set(const command_t &cmd)
  {
    if (cmd.size() != 3)
    {
      return serializer::serialize_error("ERR wrong number of arguments for 'set' command");
    }
    const std::string &key = cmd[1];
    const std::string &value = cmd[2];
    store_->set(key, value);
    return serializer::serialize_ok();
  }

  std::string command_handler::handle_del(const command_t &cmd)
  {
    if (cmd.size() < 2)
    {
      return serializer::serialize_error("ERR wrong number of arguments for 'del' command");
    }
    std::vector<std::string> keys(cmd.begin() + 1, cmd.end());
    int deleted_count = store_->del(keys);
    return serializer::serialize_integer(deleted_count);
  }

  std::string command_handler::handle_keys(const command_t &cmd)
  {
    if (cmd.size() != 2)
    {
      return serializer::serialize_error("ERR wrong number of arguments for 'keys' command");
    }

    const std::string &pattern = cmd[1];
    std::vector<std::string> matched_keys = store_->keys(pattern);

    return serializer::serialize_array(matched_keys);
  }

  std::string command_handler::handle_setex(const command_t &cmd)
  {
    if (cmd.size() != 4)
    {
      return serializer::serialize_error("ERR wrong number of arguments for 'setex' command");
    }

    const std::string &key = cmd[1];
    const std::string &ttl_str = cmd[2];
    const std::string &value = cmd[3];
    
    try
    {
      int ttl_seconds = std::stoi(ttl_str);
      if (ttl_seconds <= 0) {
        return serializer::serialize_error("ERR invalid expire time in setex");
      }
      store_->setex(key, ttl_seconds, value);
    }
    catch (const std::invalid_argument& ia)
    {
      return serializer::serialize_error("ERR value is not an integer or out of range");
    }
    catch (const std::out_of_range& oor)
    {
      return serializer::serialize_error("ERR value is not an integer or out of range");
    }

    return serializer::serialize_ok();
  }

} // namespace mini_redis
