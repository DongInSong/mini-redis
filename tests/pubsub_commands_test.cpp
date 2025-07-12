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
            srv = std::make_unique<mini_redis::server>("127.0.0.1", port);
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

TEST_F(PubSubTest, MultiChannelSubscription) {
    boost::asio::io_context io_context;
    auto sub_socket = connect_client(io_context);
    auto pub_socket = connect_client(io_context);

    // Subscribe to two channels
    write_to_socket(sub_socket, mini_redis::serializer::serialize_array({"SUBSCRIBE", "chan1", "chan2"}));
    
    // Read confirmations
    std::string resp1 = read_from_socket(sub_socket); // resp for chan1
    std::string resp2 = read_from_socket(sub_socket); // resp for chan2
    EXPECT_NE(resp1.find("chan1"), std::string::npos);
    EXPECT_NE(resp2.find("chan2"), std::string::npos);

    // Publish to chan2
    write_to_socket(pub_socket, mini_redis::serializer::serialize_array({"PUBLISH", "chan2", "hello"}));
    EXPECT_EQ(read_from_socket(pub_socket), ":1\r\n");

    // Subscriber receives message from chan2
    std::string msg = read_from_socket(sub_socket);
    EXPECT_NE(msg.find("chan2"), std::string::npos);
    EXPECT_NE(msg.find("hello"), std::string::npos);
}

TEST_F(PubSubTest, UnsubscribeFromOneOfMany) {
    boost::asio::io_context io_context;
    auto sub_socket = connect_client(io_context);
    auto pub_socket = connect_client(io_context);

    // Subscribe to two channels
    write_to_socket(sub_socket, mini_redis::serializer::serialize_array({"SUBSCRIBE", "chan1", "chan2"}));
    read_from_socket(sub_socket); // Consume "chan1" confirmation
    read_from_socket(sub_socket); // Consume "chan2" confirmation

    // Unsubscribe from "chan1"
    write_to_socket(sub_socket, mini_redis::serializer::serialize_array({"UNSUBSCRIBE", "chan1"}));
    read_from_socket(sub_socket); // Consume unsubscribe confirmation

    // Publish to "chan1" -> should go to 0 subscribers
    write_to_socket(pub_socket, mini_redis::serializer::serialize_array({"PUBLISH", "chan1", "msg1"}));
    EXPECT_EQ(read_from_socket(pub_socket), ":0\r\n");

    // Publish to "chan2" -> should go to 1 subscriber
    write_to_socket(pub_socket, mini_redis::serializer::serialize_array({"PUBLISH", "chan2", "msg2"}));
    EXPECT_EQ(read_from_socket(pub_socket), ":1\r\n");

    // Subscriber should only receive the message from "chan2"
    std::string msg = read_from_socket(sub_socket);
    EXPECT_NE(msg.find("chan2"), std::string::npos);
    EXPECT_NE(msg.find("msg2"), std::string::npos);
    EXPECT_EQ(msg.find("chan1"), std::string::npos);
}

TEST_F(PubSubTest, UnsubscribeFromAll) {
    boost::asio::io_context io_context;
    auto sub_socket = connect_client(io_context);
    auto pub_socket = connect_client(io_context);

    // Subscribe to two channels
    write_to_socket(sub_socket, mini_redis::serializer::serialize_array({"SUBSCRIBE", "chan1", "chan2"}));
    read_from_socket(sub_socket); // Consume "chan1" confirmation
    read_from_socket(sub_socket); // Consume "chan2" confirmation

    // Unsubscribe from all
    write_to_socket(sub_socket, mini_redis::serializer::serialize_array({"UNSUBSCRIBE"}));
    read_from_socket(sub_socket); // Consume "chan1" un-confirmation
    read_from_socket(sub_socket); // Consume "chan2" un-confirmation

    // Publish to "chan2" -> should go to 0 subscribers
    write_to_socket(pub_socket, mini_redis::serializer::serialize_array({"PUBLISH", "chan2", "hello"}));
    EXPECT_EQ(read_from_socket(pub_socket), ":0\r\n");
}

TEST_F(PubSubTest, MultiSubscriber) {
    boost::asio::io_context io_context;
    auto sub1_socket = connect_client(io_context);
    auto sub2_socket = connect_client(io_context);
    auto pub_socket = connect_client(io_context);

    // Both subscribe to "news"
    write_to_socket(sub1_socket, mini_redis::serializer::serialize_array({"SUBSCRIBE", "news"}));
    write_to_socket(sub2_socket, mini_redis::serializer::serialize_array({"SUBSCRIBE", "news"}));
    read_from_socket(sub1_socket); // Consume sub1 confirmation
    read_from_socket(sub2_socket); // Consume sub2 confirmation

    // Publish to "news"
    write_to_socket(pub_socket, mini_redis::serializer::serialize_array({"PUBLISH", "news", "breaking"}));
    EXPECT_EQ(read_from_socket(pub_socket), ":2\r\n");

    // Check both subscribers received it
    std::string msg1 = read_from_socket(sub1_socket);
    std::string msg2 = read_from_socket(sub2_socket);
    EXPECT_NE(msg1.find("breaking"), std::string::npos);
    EXPECT_NE(msg2.find("breaking"), std::string::npos);
}

TEST_F(PubSubTest, NoSubscribers) {
    boost::asio::io_context io_context;
    auto pub_socket = connect_client(io_context);

    // Publish to a channel with no subscribers
    write_to_socket(pub_socket, mini_redis::serializer::serialize_array({"PUBLISH", "lonely-channel", "echo"}));
    EXPECT_EQ(read_from_socket(pub_socket), ":0\r\n");
}

TEST_F(PubSubTest, InvalidCommandInSubMode) {
    boost::asio::io_context io_context;
    auto sub_socket = connect_client(io_context);

    // Subscribe to enter pub/sub mode
    write_to_socket(sub_socket, mini_redis::serializer::serialize_array({"SUBSCRIBE", "news"}));
    read_from_socket(sub_socket); // Consume confirmation

    // Try to execute a non-pub/sub command
    write_to_socket(sub_socket, mini_redis::serializer::serialize_array({"SET", "key", "value"}));
    
    // Expect an error
    std::string err_resp = read_from_socket(sub_socket);
    EXPECT_EQ(err_resp.substr(0, 1), "-"); // Should start with '-' for error
    EXPECT_NE(err_resp.find("allowed in this context"), std::string::npos);
}
