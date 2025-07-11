#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include "network/server.hpp"
#include "protocol/serializer.hpp"

using boost::asio::ip::tcp;

class PubSubTest : public ::testing::Test {
protected:
    std::unique_ptr<mini_redis::server> srv;
    std::thread srv_thread;
    const short port = 16379; // Use a different port for testing

    void SetUp() override {
        try {
            srv = std::make_unique<mini_redis::server>(port);
            srv_thread = std::thread([this]() {
                srv->run();
            });
            // Give the server a moment to start up
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (const std::exception& e) {
            FAIL() << "Failed to set up server: " << e.what();
        }
    }

    void TearDown() override {
        srv->stop();
        if (srv_thread.joinable()) {
            srv_thread.join();
        }
    }

    // Helper to connect a client
    tcp::socket connect_client(boost::asio::io_context& io_context) {
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", std::to_string(port)));
        return socket;
    }

    // Helper to write data to socket
    void write_to_socket(tcp::socket& socket, const std::string& data) {
        boost::asio::write(socket, boost::asio::buffer(data));
    }

    // Helper to read data from socket
    std::string read_from_socket(tcp::socket& socket) {
        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, "\r\n");
        return std::string(boost::asio::buffers_begin(buf.data()), boost::asio::buffers_end(buf.data()));
    }
};

TEST_F(PubSubTest, SingleSubscriberReceivesMessage) {
    boost::asio::io_context io_context_sub, io_context_pub;

    // Subscriber client
    auto subscriber_socket = connect_client(io_context_sub);
    
    // Publisher client
    auto publisher_socket = connect_client(io_context_pub);

    // 1. Subscriber subscribes to "channel1"
    std::string subscribe_cmd = mini_redis::serializer::serialize_array({"SUBSCRIBE", "channel1"});
    write_to_socket(subscriber_socket, subscribe_cmd);

    // 2. Read subscribe confirmation
    std::string sub_response = read_from_socket(subscriber_socket);
    // Expected: *2\r\n$9\r\nsubscribe\r\n$8\r\nchannel1\r\n
    // We just check for the subscribe keyword for simplicity
    EXPECT_NE(sub_response.find("subscribe"), std::string::npos);
    EXPECT_NE(sub_response.find("channel1"), std::string::npos);

    // 3. Publisher publishes a message to "channel1"
    std::string message = "hello world";
    std::string publish_cmd = mini_redis::serializer::serialize_array({"PUBLISH", "channel1", message});
    write_to_socket(publisher_socket, publish_cmd);

    // 4. Read publish response (number of clients that received the message)
    std::string pub_response = read_from_socket(publisher_socket);
    EXPECT_EQ(pub_response, ":1\r\n");

    // 5. Subscriber reads the published message
    std::string received_message = read_from_socket(subscriber_socket);
    // Expected: *3\r\n$7\r\nmessage\r\n$8\r\nchannel1\r\n$11\r\nhello world\r\n
    EXPECT_NE(received_message.find("message"), std::string::npos);
    EXPECT_NE(received_message.find("channel1"), std::string::npos);
    EXPECT_NE(received_message.find(message), std::string::npos);
}
