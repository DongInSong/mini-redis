#ifndef MINI_REDIS_SESSION_HPP
#define MINI_REDIS_SESSION_HPP

#include <boost/asio.hpp>
#include <memory>
#include "protocol/parser.hpp"
#include "command/handler.hpp"
#include "storage/store.hpp"

namespace mini_redis
{
  class pubsub_manager; // Forward declaration

  class session : public std::enable_shared_from_this<session>
  {
  public:
    session(boost::asio::ip::tcp::socket socket, std::shared_ptr<store> s, std::shared_ptr<pubsub_manager> ps_manager);
    ~session();
    void start();
    void deliver(const std::string &msg);

  private:
    void do_read();
    void do_write(std::string response);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf read_buffer_;
    parser parser_;
    command_handler handler_;
    std::shared_ptr<pubsub_manager> pubsub_manager_;
  };
} // namespace mini_redis

#endif // MINI_REDIS_SESSION_HPP
