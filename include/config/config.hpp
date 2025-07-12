#ifndef MINI_REDIS_CONFIG_HPP
#define MINI_REDIS_CONFIG_HPP

#include <string>
#include <yaml-cpp/yaml.h>

namespace mini_redis
{
    class Config
    {
    public:
        Config(const std::string& filename);

        std::string get_host() const;
        short get_port() const;

    private:
        YAML::Node config_node_;
        YAML::Node get_server_node() const;
    };
} // namespace mini_redis

#endif // MINI_REDIS_CONFIG_HPP
