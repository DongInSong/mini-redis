#include "config/config.hpp"
#include <stdexcept>

namespace mini_redis
{
    Config::Config(const std::string& filename)
    {
        try {
            config_node_ = YAML::LoadFile(filename);
        } catch (const YAML::Exception& e) {
            throw std::runtime_error("Failed to load or parse config file: " + filename + " - " + e.what());
        }
    }
    
    YAML::Node Config::get_server_node() const
    {
        if (config_node_["server"]) {
            return config_node_["server"];
        }
        throw std::runtime_error("Server configuration not found in config file.");
    }

    short Config::get_port() const
    {
        YAML::Node server = get_server_node();
        if (server["port"] && server["port"].IsScalar())
        {
            return server["port"].as<short>();
        }
        // Default port if not specified
        return 6379;
    }

    std::string Config::get_host() const
    {
        YAML::Node server = get_server_node();
        if (server["host"] && server["host"].IsScalar())
        {
            return server["host"].as<std::string>();
        }
        // Default host if not specified
        return "0.0.0.0";
    }
} // namespace mini_redis
