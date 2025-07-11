#include "network/server.hpp"
#include "network/session.hpp"
#include <iostream>
#include <thread>
#include <memory>
#include <algorithm>

namespace mini_redis
{
  server::server(short port)
      : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        store_(std::make_shared<store>()),
        pubsub_manager_(std::make_shared<pubsub_manager>())
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
      std::make_shared<session>(std::move(socket), store_, pubsub_manager_)->start();
    }
    else
    {
      std::cerr << "Accept error: " << error.message() << std::endl;
    }
    start_accept();
  }
} // namespace mini_redis
