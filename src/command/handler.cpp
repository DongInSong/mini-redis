#include "command/handler.hpp"
#include "network/session.hpp"
#include "protocol/serializer.hpp"
#include <algorithm> 
#include <cctype> 

namespace mini_redis
{
  command_handler::command_handler(std::shared_ptr<store> store, std::shared_ptr<pubsub_manager> pubsub_manager) 
    : store_(store), pubsub_manager_(pubsub_manager) {}

  void command_handler::set_session(std::weak_ptr<session> s) {
    session_ = s;
  }

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
    else if (command_name == "SUBSCRIBE")
    {
      return handle_subscribe(cmd);
    }
    else if (command_name == "UNSUBSCRIBE")
    {
      return handle_unsubscribe(cmd);
    }
    else if (command_name == "PUBLISH")
    {
      return handle_publish(cmd);
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
    catch (const std::invalid_argument&)
    {
      return serializer::serialize_error("ERR value is not an integer or out of range");
    }
    catch (const std::out_of_range&)
    {
      return serializer::serialize_error("ERR value is not an integer or out of range");
    }

    return serializer::serialize_ok();
  }

  std::string command_handler::handle_subscribe(const command_t &cmd)
  {
    if (cmd.size() < 2) {
      return serializer::serialize_error("ERR wrong number of arguments for 'subscribe' command");
    }

    if (auto s = session_.lock()) {
      for (size_t i = 1; i < cmd.size(); ++i) {
        const auto& channel = cmd[i];
        s->subscribe_to_channel(channel);
        s->send_pubsub_response("subscribe", channel, static_cast<int>(s->get_subscription_count()));
      }
    }

    return ""; 
  }

  std::string command_handler::handle_unsubscribe(const command_t &cmd)
  {
    if (auto s = session_.lock()) {
      if (cmd.size() == 1) {
        // Unsubscribe from all channels
        auto subscribed_channels = s->get_subscribed_channels();
        if (subscribed_channels.empty()) {
            // Not subscribed to any channels, but send a confirmation anyway
            s->send_pubsub_response("unsubscribe", "", static_cast<int>(s->get_subscription_count()));
        } else {
            for (const auto& channel : subscribed_channels) {
                s->unsubscribe_from_channel(channel);
                s->send_pubsub_response("unsubscribe", channel, static_cast<int>(s->get_subscription_count()));
            }
        }
      } else {
        for (size_t i = 1; i < cmd.size(); ++i) {
          const auto& channel = cmd[i];
          s->unsubscribe_from_channel(channel);
          s->send_pubsub_response("unsubscribe", channel, static_cast<int>(s->get_subscription_count()));
        }
      }
    }
    return "";
  }

  std::string command_handler::handle_publish(const command_t &cmd)
  {
    if (cmd.size() != 3) {
      return serializer::serialize_error("ERR wrong number of arguments for 'publish' command");
    }

    const std::string& channel = cmd[1];
    const std::string& message = cmd[2];

    int receivers = pubsub_manager_->publish(channel, message);
    return serializer::serialize_integer(receivers);
  }

} // namespace mini_redis
