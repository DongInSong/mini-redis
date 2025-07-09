#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// 명령어를 RESP 형식으로 인코딩하는 함수
std::string encode_resp_command(const std::vector<std::string>& command) {
    std::string encoded = "*" + std::to_string(command.size()) + "\r\n";
    for (const auto& arg : command) {
        encoded += "$" + std::to_string(arg.length()) + "\r\n";
        encoded += arg + "\r\n";
    }
    return encoded;
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("localhost", "6379");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to mini-redis server." << std::endl;
        std::cout << "Example commands: PING, PING [message], SET key value, GET key" << std::endl;
        
        for (std::string line; std::cout << "> " && std::getline(std::cin, line);) {
            if (line.empty()) continue;

            // 간단한 공백 기반 파싱
            std::vector<std::string> cmd_parts;
            std::string current_part;
            std::istringstream iss(line);
            while (iss >> current_part) {
                cmd_parts.push_back(current_part);
            }

            if (cmd_parts.empty()) continue;

            // RESP 형식으로 인코딩
            std::string request = encode_resp_command(cmd_parts);

            // 서버에 전송
            boost::asio::write(socket, boost::asio::buffer(request));

            // 서버로부터 응답 읽기
            boost::asio::streambuf response_buf;
            boost::system::error_code error;
            size_t len = boost::asio::read_until(socket, response_buf, "\r\n", error);

            if (error == boost::asio::error::eof) {
                std::cout << "Connection closed by server." << std::endl;
                break;
            } else if (error) {
                throw boost::system::system_error(error);
            }

            std::string response(boost::asio::buffers_begin(response_buf.data()), boost::asio::buffers_begin(response_buf.data()) + len);
            response_buf.consume(len);

            // 응답 출력 (단순 문자열, 벌크 문자열 등 간단한 케이스만 처리)
            if (response.front() == '+') {
                std::cout << response.substr(1, response.size() - 3) << std::endl;
            } else if (response.front() == '-') {
                std::cout << "(error) " << response.substr(1, response.size() - 3) << std::endl;
            } else if (response.front() == '$') {
                // 벌크 문자열 길이 읽기
                size_t str_len = std::stoi(response.substr(1, response.find("\r\n") - 1));
                // 나머지 데이터 읽기
                boost::asio::read(socket, response_buf, boost::asio::transfer_exactly(str_len + 2), error);
                std::string bulk_str(boost::asio::buffers_begin(response_buf.data()), boost::asio::buffers_begin(response_buf.data()) + str_len);
                response_buf.consume(str_len + 2);
                std::cout << '"' << bulk_str << '"' << std::endl;
            } else {
                 std::cout << response;
            }
        }

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
