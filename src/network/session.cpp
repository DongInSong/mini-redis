#include "network/session.hpp"
#include "pubsub/manager.hpp"
#include <iostream>
#include "network/session.hpp"
#include "protocol/serializer.hpp"
#include <iostream>
#include <vector>

namespace mini_redis
{
  session::session(boost::asio::ip::tcp::socket socket, std::shared_ptr<store> s, std::shared_ptr<pubsub_manager> ps_manager)
      : socket_(std::move(socket)), handler_(s, ps_manager), pubsub_manager_(ps_manager)
  {
  }

  session::~session()
  {
    // When a session is destroyed, unsubscribe it from all channels.
    pubsub_manager_->unsubscribe_all(this);
  }

  void session::start()
  {
    // weak ptr를 사용하여 세션을 핸들러에 설정
    // This allows the handler to access the session without taking ownership.
    // The session will be destroyed when the socket is closed or an error occurs.
    // This prevents circular references and memory leaks.
    handler_.set_session(weak_from_this());
    do_read();
  }

  void session::deliver(const std::string &msg)
  {
    do_write(msg);
  }

  void session::subscribe_to_channel(const std::string& channel)
  {
    pubsub_manager_->subscribe(channel, shared_from_this());
    subscribed_channels_.insert(channel);
  }

  void session::unsubscribe_from_channel(const std::string& channel)
  {
    pubsub_manager_->unsubscribe(channel, this);
    subscribed_channels_.erase(channel);
  }

  size_t session::get_subscription_count() const
  {
    return subscribed_channels_.size();
  }

  std::set<std::string> session::get_subscribed_channels() const
  {
    return subscribed_channels_;
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
            std::string upper_cmd = cmd[0];
            std::transform(upper_cmd.begin(), upper_cmd.end(), upper_cmd.begin(), ::toupper);

            if (is_subscribed() && (upper_cmd != "SUBSCRIBE" && upper_cmd != "UNSUBSCRIBE" && upper_cmd != "PING")) {
                do_write(serializer::serialize_error("ERR only 'SUBSCRIBE', 'UNSUBSCRIBE', and 'PING' are allowed in this context"));
                continue;
            }
            std::string result = handler_.execute_command(cmd);
            if (!result.empty()) {
              do_write(result);
            }
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

  void session::do_write(const std::string& response)
  {
    write_queue_.push_back(response);
    if (!writing_in_progress_) {
      do_queued_write();
    }
  }

  void session::do_queued_write()
  {
    if (write_queue_.empty()) {
      writing_in_progress_ = false;
      return;
    }

    writing_in_progress_ = true;
    auto self = shared_from_this();
    boost::asio::async_write(socket_, boost::asio::buffer(write_queue_.front()),
      [this, self](const boost::system::error_code &ec, std::size_t) {
        if (!ec) {
          write_queue_.pop_front();
          do_queued_write();
        } else {
          std::cerr << "Write error: " << ec.message() << std::endl;
          writing_in_progress_ = false;
        }
      });
  }

  void session::send_pubsub_response(const std::string &type, const std::string &channel, std::optional<int> subscription_count)
  {
    std::vector<std::string> response_parts;
    response_parts.push_back(type);
    response_parts.push_back(channel);
    if (subscription_count.has_value()) {
      response_parts.push_back(std::to_string(subscription_count.value()));
    }

    std::string serialized_response = serializer::serialize_array(response_parts);
    deliver(serialized_response);
  }
}
