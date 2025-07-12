#ifndef MINI_REDIS_I_COMMAND_HANDLER_HPP
#define MINI_REDIS_I_COMMAND_HANDLER_HPP

#include "protocol/parser.hpp"
#include <string>
#include <vector>
#include <memory>

namespace mini_redis
{
    class ICommandHandler
    {
    public:
        virtual ~ICommandHandler() = default;
        virtual bool supports(const std::string& command_name) const = 0;
        virtual std::string execute(const command_t& cmd) = 0;
    };
} // namespace mini_redis

#endif // MINI_REDIS_I_COMMAND_HANDLER_HPP
