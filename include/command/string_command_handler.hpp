#ifndef MINI_REDIS_STRING_COMMAND_HANDLER_HPP
#define MINI_REDIS_STRING_COMMAND_HANDLER_HPP

#include "command/command_handler_interface.hpp"
#include "storage/store.hpp"
#include <memory>

namespace mini_redis
{
    class StringCommandHandler : public ICommandHandler
    {
    public:
        explicit StringCommandHandler(std::shared_ptr<store> store);
        bool supports(const std::string& command_name) const override;
        std::string execute(const command_t& cmd) override;

    private:
        std::shared_ptr<store> store_;
        std::string handle_get(const command_t &cmd);
        std::string handle_set(const command_t &cmd);
        std::string handle_setex(const command_t &cmd);
    };
} // namespace mini_redis

#endif // MINI_REDIS_STRING_COMMAND_HANDLER_HPP
