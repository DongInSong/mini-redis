#include "network/server.hpp"
#include "command/handler.hpp"
#include "protocol/parser.hpp"
#include <iostream>
#include <thread>
#include <memory>
#include <algorithm>

namespace mini_redis
{
  // 세션 클래스 나중에 분리
  class session : public std::enable_shared_from_this<session>
  {
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf read_buffer_;
    parser parser_;
    command_handler handler_;

  public:
    session(boost::asio::ip::tcp::socket socket, std::shared_ptr<store> s) 
      : socket_(std::move(socket)), handler_(s) {}

    // 클라이언트 읽기
    void start() {
      do_read();
    }

  private:
    void do_read() {
        auto self = shared_from_this();
        // 비동기적 데이터 읽기
        socket_.async_read_some(read_buffer_.prepare(1024),
            [this, self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    read_buffer_.commit(bytes_transferred);
                    // streambuf -> string 데이터 변환
                    std::string data(boost::asio::buffers_begin(read_buffer_.data()), 
                                     boost::asio::buffers_begin(read_buffer_.data()) + bytes_transferred);
                    read_buffer_.consume(bytes_transferred);

                    // resp 명령어 파싱 및 실행
                    auto commands = parser_.parse(data);
                    for (const auto& cmd : commands) {
                        std::string result = handler_.execute_command(cmd);
                        do_write(result);
                    }
                    // 읽기
                    do_read();
                } else if (ec != boost::asio::error::eof) {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
            });
    }

    void do_write(std::string response) {
      auto self = shared_from_this();
      auto response_ptr = std::make_shared<std::string>(std::move(response));
      boost::asio::async_write(socket_, boost::asio::buffer(*response_ptr),
        [this, self, response_ptr](const boost::system::error_code& ec, std::size_t) {
          if (ec) {
            std::cerr << "Write error: " << ec.message() << std::endl;
          }
        });
    }
  };

  server::server(short port)
      : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
  {
    // 생성자 연결
    start_accept();
  }

  void server::run()
  {
    // CPU 코어 수만큼 스레드 풀 생성
    const auto thread_count = std::max<int>(1, std::thread::hardware_concurrency());
    for (int i = 0; i < thread_count; ++i) {
      thread_pool_.emplace_back([this] {
        try {
        io_context_.run();
        } catch (const std::exception &e) {
          std::cerr << "Thread exception: " << e.what() << std::endl;
        }
      });  
    }
    // 메인 스레드 - 스레드 최소 2개
    try {
      io_context_.run();
    } catch (const std::exception &e) {
      std::cerr << "Main thread exception: " << e.what() << std::endl;
    }
  }


  void server::stop()
  {
    std::cout << "Stopping server..." << std::endl;
    // I/O 서비스를 중지하고 모든 스레드가 종료될 때까지 대기
    io_context_.stop();
    for (auto& t : thread_pool_) {
      if (t.joinable()) {
        t.join();
      }
    }
    std::cout << "Server stopped." << std::endl;
  }

  void server::start_accept()
  {
    // 비동기적 새 클라이언트 연결 수락
    acceptor_.async_accept(
        [this](const boost::system::error_code &error, boost::asio::ip::tcp::socket socket) {
          handle_accept(std::move(socket), error);
        });
  }

  void server::handle_accept(boost::asio::ip::tcp::socket &&socket, const boost::system::error_code &error)
  {
    if (!error)
    {
      // 데이터 저장소를 생성하고 세션에 전달
      auto s = std::make_shared<store>();
      std::make_shared<session>(std::move(socket), s)->start();
    } else {
      std::cerr << "Accept error: " << error.message() << std::endl;
    }
    start_accept();
  }
} // namespace mini_redis
