#include "command/generic_command_handler.hpp"
#include "protocol/serializer.hpp"
#include <algorithm>
#include <cctype>

namespace mini_redis
{
    GenericCommandHandler::GenericCommandHandler(std::shared_ptr<store> store) : store_(store) {}

    bool GenericCommandHandler::supports(const std::string& command_name) const {
        std::string upper_cmd = command_name;
        std::transform(upper_cmd.begin(), upper_cmd.end(), upper_cmd.begin(), ::toupper);
        return upper_cmd == "PING" || upper_cmd == "DEL" || upper_cmd == "KEYS";
    }

    std::string GenericCommandHandler::execute(const command_t& cmd) {
        std::string command_name = cmd[0];
        std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);

        if (command_name == "PING") {
            return handle_ping(cmd);
        } else if (command_name == "DEL") {
            return handle_del(cmd);
        } else if (command_name == "KEYS") {
            return handle_keys(cmd);
        }
        return serializer::serialize_error("ERR unknown command `" + cmd[0] + "`");
    }

    std::string GenericCommandHandler::handle_ping(const command_t &cmd)
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

    std::string GenericCommandHandler::handle_del(const command_t &cmd)
    {
        if (cmd.size() < 2)
        {
            return serializer::serialize_error("ERR wrong number of arguments for 'del' command");
        }
        std::vector<std::string> keys(cmd.begin() + 1, cmd.end());
        int deleted_count = store_->del(keys);
        return serializer::serialize_integer(deleted_count);
    }

    std::string GenericCommandHandler::handle_keys(const command_t &cmd)
    {
        if (cmd.size() != 2)
        {
            return serializer::serialize_error("ERR wrong number of arguments for 'keys' command");
        }

        const std::string &pattern = cmd[1];
        std::vector<std::string> matched_keys = store_->keys(pattern);

        return serializer::serialize_array(matched_keys);
    }
} // namespace mini_redis
