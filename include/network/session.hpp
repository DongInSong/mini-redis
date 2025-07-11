#ifndef MINI_REDIS_SESSION_HPP
#define MINI_REDIS_SESSION_HPP

#include <boost/asio.hpp>
#include <memory>
#include <set>
#include <deque>
#include <optional>
#include "protocol/parser.hpp"
#include "command/handler.hpp"
#include "storage/store.hpp"

namespace mini_redis
{
  class pubsub_manager; // Forward declaration

  class session : public std::enable_shared_from_this<session>
  {
    friend class pubsub_manager;
  public:
    session(boost::asio::ip::tcp::socket socket, std::shared_ptr<store> s, std::shared_ptr<pubsub_manager> ps_manager);
    ~session();
    void start();
    void deliver(const std::string &msg);

  public:
    void subscribe_to_channel(const std::string& channel);
    void unsubscribe_from_channel(const std::string& channel);
    size_t get_subscription_count() const;
    std::set<std::string> get_subscribed_channels() const;
    void send_pubsub_response(const std::string &type, const std::string &channel, std::optional<int> subscription_count);

  private:
    void do_read();
    void do_write(const std::string& response);
    void do_queued_write();

    bool is_subscribed() const { return !subscribed_channels_.empty(); }

    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf read_buffer_;
    parser parser_;
    command_handler handler_;
    std::shared_ptr<pubsub_manager> pubsub_manager_;
    
    // Pub/Sub state
    std::set<std::string> subscribed_channels_;

    // Write queue to ensure sequential writes
    std::deque<std::string> write_queue_;
    bool writing_in_progress_ = false;
  };
} // namespace mini_redis

#endif // MINI_REDIS_SESSION_HPP
