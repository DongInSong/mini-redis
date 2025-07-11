#include "network/session.hpp"
#include "pubsub/manager.hpp"
#include <iostream>

namespace mini_redis
{
  session::session(boost::asio::ip::tcp::socket socket, std::shared_ptr<store> s, std::shared_ptr<pubsub_manager> ps_manager)
      : socket_(std::move(socket)),
        handler_(s),
        pubsub_manager_(ps_manager)
  {
  }

  session::~session()
  {
    // When a session is destroyed, unsubscribe it from all channels.
    pubsub_manager_->unsubscribe_all(shared_from_this());
  }

  void session::start()
  {
    do_read();
  }

  void session::deliver(const std::string &msg)
  {
    do_write(msg);
  }

  void session::do_read()
  {
    auto self = shared_from_this();
    // 비동기적 데이터 읽기
    socket_.async_read_some(read_buffer_.prepare(1024),
      [this, self](const boost::system::error_code &ec, std::size_t bytes_transferred) {
        if (!ec)
        {
          read_buffer_.commit(bytes_transferred);
          // streambuf -> string 데이터 변환
          std::string data(boost::asio::buffers_begin(read_buffer_.data()),
                           boost::asio::buffers_begin(read_buffer_.data()) + bytes_transferred);
          read_buffer_.consume(bytes_transferred);

          // resp 명령어 파싱 및 실행
          auto commands = parser_.parse(data);
          for (const auto &cmd : commands)
          {
            // TODO: This part needs to be updated to handle pub/sub commands.
            // For example, if the client is in a subscribed state, it should not process other commands.
            // The command handler also needs to be aware of the session to call pubsub_manager.
            std::string result = handler_.execute_command(cmd);
            std::cout << "Command executed: " << cmd[0] << std::endl;
            std::cout << "Response: " << result << std::endl;
            do_write(result);
          }
          // 읽기
          do_read();
        }
        else if (ec != boost::asio::error::eof)
        {
          std::cerr << "Read error: " << ec.message() << std::endl;
        }
        // On error or EOF, the session object will be destroyed, and the destructor will handle cleanup.
      });
  }
}