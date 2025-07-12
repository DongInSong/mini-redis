#include "command/string_command_handler.hpp"
#include "protocol/serializer.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace mini_redis
{
    StringCommandHandler::StringCommandHandler(std::shared_ptr<store> store) : store_(store) {}

    bool StringCommandHandler::supports(const std::string& command_name) const {
        std::string upper_cmd = command_name;
        std::transform(upper_cmd.begin(), upper_cmd.end(), upper_cmd.begin(), ::toupper);
        return upper_cmd == "GET" || upper_cmd == "SET" || upper_cmd == "SETEX";
    }

    std::string StringCommandHandler::execute(const command_t& cmd) {
        std::string command_name = cmd[0];
        std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);

        if (command_name == "GET") {
            return handle_get(cmd);
        } else if (command_name == "SET") {
            return handle_set(cmd);
        } else if (command_name == "SETEX") {
            return handle_setex(cmd);
        }
        return serializer::serialize_error("ERR unknown command `" + cmd[0] + "`");
    }

    std::string StringCommandHandler::handle_get(const command_t &cmd)
    {
        if (cmd.size() != 2)
        {
            return serializer::serialize_error("ERR wrong number of arguments for 'get' command");
        }
        const std::string &key = cmd[1];
        std::optional<std::string> value = store_->get(key);
        if (value)
        {
            return serializer::serialize_bulk_string(*value);
        }
        else
        {
            return serializer::serialize_null_bulk_string();
        }
    }

    std::string StringCommandHandler::handle_set(const command_t &cmd)
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

    std::string StringCommandHandler::handle_setex(const command_t &cmd)
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
} // namespace mini_redis
