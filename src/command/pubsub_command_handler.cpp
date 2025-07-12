#include "command/pubsub_command_handler.hpp"
#include "protocol/serializer.hpp"
#include <algorithm>
#include <cctype>

namespace mini_redis
{
    PubSubCommandHandler::PubSubCommandHandler(std::shared_ptr<pubsub_manager> pubsub_manager) : pubsub_manager_(pubsub_manager) {}

    void PubSubCommandHandler::set_session(std::weak_ptr<session> s) {
        session_ = s;
    }

    bool PubSubCommandHandler::supports(const std::string& command_name) const {
        std::string upper_cmd = command_name;
        std::transform(upper_cmd.begin(), upper_cmd.end(), upper_cmd.begin(), ::toupper);
        return upper_cmd == "SUBSCRIBE" || upper_cmd == "UNSUBSCRIBE" || upper_cmd == "PUBLISH";
    }

    std::string PubSubCommandHandler::execute(const command_t& cmd) {
        std::string command_name = cmd[0];
        std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);

        if (command_name == "SUBSCRIBE") {
            return handle_subscribe(cmd);
        } else if (command_name == "UNSUBSCRIBE") {
            return handle_unsubscribe(cmd);
        } else if (command_name == "PUBLISH") {
            return handle_publish(cmd);
        }
        return serializer::serialize_error("ERR unknown command `" + cmd[0] + "`");
    }

    std::string PubSubCommandHandler::handle_subscribe(const command_t &cmd)
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

    std::string PubSubCommandHandler::handle_unsubscribe(const command_t &cmd)
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

    std::string PubSubCommandHandler::handle_publish(const command_t &cmd)
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
