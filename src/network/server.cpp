#include "network/server.hpp"
#include "command/handler.hpp"
#include "protocol/parser.hpp"
#include <iostream>
#include <thread>

namespace mini_redis
{
  server::server(short port)
      : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
  {
    // TODO: Implement constructor
  }

  void server::run()
  {
    // TODO: Implement run logic
    // 1. Start accepting connections
    // 2. Run io_context in a thread pool
  }

  void server::stop()
  {
    // TODO: Implement stop logic
    // 1. Stop the io_context
    // 2. Join all threads in the pool
  }

  void server::start_accept()
  {
    // TODO: Implement accept logic
    // 1. Create a new socket
    // 2. Asynchronously accept a new connection
  }

  void server::handle_accept(boost::asio::ip::tcp::socket &&new_connection, const boost::system::error_code &error)
  {
    // TODO: Implement handle_accept logic
    // 1. Check for errors
    // 2. If no error, create a session/handler for the new connection
    // 3. Start accepting the next connection
  }
} // namespace mini_redis
