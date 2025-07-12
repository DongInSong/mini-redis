#ifndef MINI_REDIS_SERVER_HPP
#define MINI_REDIS_SERVER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <thread>
#include "storage/store.hpp"
#include "pubsub/manager.hpp"

namespace mini_redis
{
  class server
  {
  public:
    /**
     * @brief Construct a new server object
     * 
     * @param host The host address to listen on
     * @param port The port to listen on
     */
    server(const std::string& host, short port);

    /**
     * @brief Runs the server's I/O service loop.
     * This function will block until the server is stopped.
     */
    void run();

    /**
     * @brief Stops the server.
     */
    void stop();

  private:
    /**
     * @brief Starts accepting incoming connections.
     */
    void start_accept();

    /**
     * @brief Handles a new connection.
     * 
     * @param new_connection The new connection socket.
     * @param error The error code, if any.
     */
    void handle_accept(boost::asio::ip::tcp::socket &&new_connection, const boost::system::error_code &error);

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> thread_pool_;
    std::shared_ptr<store> store_;
    std::shared_ptr<pubsub_manager> pubsub_manager_;
  };
} // namespace mini_redis

#endif // MINI_REDIS_SERVER_HPP
