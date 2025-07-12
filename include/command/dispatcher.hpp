#ifndef MINI_REDIS_DISPATCHER_HPP
#define MINI_REDIS_DISPATCHER_HPP

#include "storage/store.hpp"
#include "pubsub/manager.hpp"
#include "command/command_handler_interface.hpp"
#include <vector>
#include <memory>

namespace mini_redis
{
    class session; // Forward declaration

    class CommandDispatcher
    {
    public:
        CommandDispatcher(std::shared_ptr<store> store, std::shared_ptr<pubsub_manager> pubsub_manager);
        void set_session(std::weak_ptr<session> s);
        std::string execute_command(const command_t &cmd);

    private:
        std::vector<std::unique_ptr<ICommandHandler>> handlers_;
    };
} // namespace mini_redis

#endif // MINI_REDIS_DISPATCHER_HPP
