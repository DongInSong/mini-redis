#ifndef MINI_REDIS_PUBSUB_COMMAND_HANDLER_HPP
#define MINI_REDIS_PUBSUB_COMMAND_HANDLER_HPP

#include "command/command_handler_interface.hpp"
#include "pubsub/manager.hpp"
#include "network/session.hpp"
#include <memory>

namespace mini_redis
{
    class PubSubCommandHandler : public ICommandHandler
    {
    public:
        PubSubCommandHandler(std::shared_ptr<pubsub_manager> pubsub_manager);
        void set_session(std::weak_ptr<session> s);
        bool supports(const std::string& command_name) const override;
        std::string execute(const command_t& cmd) override;

    private:
        std::shared_ptr<pubsub_manager> pubsub_manager_;
        std::weak_ptr<session> session_;
        std::string handle_subscribe(const command_t &cmd);
        std::string handle_unsubscribe(const command_t &cmd);
        std::string handle_publish(const command_t &cmd);
    };
} // namespace mini_redis

#endif // MINI_REDIS_PUBSUB_COMMAND_HANDLER_HPP
