#ifndef MINI_REDIS_HANDLER_HPP
#define MINI_REDIS_HANDLER_HPP

#include "storage/store.hpp"
#include "protocol/parser.hpp"
#include <string>
#include <memory>

namespace mini_redis
{
  class command_handler
  {
  public:
    /**
     * @brief Construct a new command handler object
     * 
     * @param store A shared pointer to the data store.
     */
    command_handler(std::shared_ptr<store> store);

    /**
     * @brief Executes a command and returns the result.
     * 
     * @param cmd The command to execute.
     * @return The result of the command, formatted as a RESP string.
     */
    std::string execute_command(const command_t &cmd);

  private:
    std::shared_ptr<store> store_;

    // Command-specific handler functions
    std::string handle_get(const command_t &cmd);
    std::string handle_set(const command_t &cmd);
    std::string handle_del(const command_t &cmd);
  };
} // namespace mini_redis

#endif // MINI_REDIS_HANDLER_HPP
