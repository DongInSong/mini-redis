#ifndef MINI_REDIS_GENERIC_COMMAND_HANDLER_HPP
#define MINI_REDIS_GENERIC_COMMAND_HANDLER_HPP

#include "command/command_handler_interface.hpp"
#include "storage/store.hpp"
#include <memory>

namespace mini_redis
{
    class GenericCommandHandler : public ICommandHandler
    {
    public:
        explicit GenericCommandHandler(std::shared_ptr<store> store);
        bool supports(const std::string& command_name) const override;
        std::string execute(const command_t& cmd) override;

    private:
        std::shared_ptr<store> store_;
        std::string handle_ping(const command_t &cmd);
        std::string handle_del(const command_t &cmd);
        std::string handle_keys(const command_t &cmd);
    };
} // namespace mini_redis

#endif // MINI_REDIS_GENERIC_COMMAND_HANDLER_HPP
