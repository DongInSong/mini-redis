#include "command/handler.hpp"
#include <stdexcept>

namespace mini_redis
{
  command_handler::command_handler(std::shared_ptr<store> store) : store_(store)
  {
    // Constructor implementation
  }

  std::string command_handler::execute_command(const command_t &cmd)
  {
    // TODO: Implement command dispatching
    // 1. Validate the command size.
    // 2. Convert the command name to lowercase for case-insensitive matching.
    // 3. Dispatch to the appropriate handler (handle_get, handle_set, etc.).
    // 4. Return an error message for unknown commands.

    if (cmd.empty())
    {
      return "-ERR wrong number of arguments\r\n";
    }

    const auto &command_name = cmd[0];
    if (command_name == "GET" || command_name == "get")
    {
      return handle_get(cmd);
    }
    else if (command_name == "SET" || command_name == "set")
    {
      return handle_set(cmd);
    }
    else if (command_name == "DEL" || command_name == "del")
    {
      return handle_del(cmd);
    }

    return "-ERR unknown command\r\n";
  }

  std::string command_handler::handle_get(const command_t &cmd)
  {
    // TODO: Implement GET command logic
    // 1. Check for the correct number of arguments.
    // 2. Call store_->get().
    // 3. Format the result as a RESP bulk string or nil.
    return "$-1\r\n"; // Dummy nil response
  }

  std::string command_handler::handle_set(const command_t &cmd)
  {
    // TODO: Implement SET command logic
    // 1. Check for the correct number of arguments.
    // 2. Call store_->set().
    // 3. Return "+OK\r\n".
    return "+OK\r\n"; // Dummy OK response
  }

  std::string command_handler::handle_del(const command_t &cmd)
  {
    // TODO: Implement DEL command logic
    // 1. Check for the correct number of arguments.
    // 2. Call store_->del().
    // 3. Return the number of deleted keys as a RESP integer.
    return ":0\r\n"; // Dummy response
  }

} // namespace mini_redis
