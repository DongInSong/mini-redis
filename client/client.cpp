#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

void handle_response_recursive(tcp::socket& socket, boost::asio::streambuf& response_buf);

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
        std::cout << "Example commands: PING, SET key value, GET key, DEL key, KEYS *" << std::endl;
        
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

            // 서버로부터 응답 읽기 및 처리
            boost::asio::streambuf response_buf;
            handle_response_recursive(socket, response_buf);
        }

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

void handle_response_recursive(tcp::socket& socket, boost::asio::streambuf& response_buf) {
    boost::system::error_code error;
    
    if (response_buf.size() == 0) {
        boost::asio::read_until(socket, response_buf, "\r\n", error);
        if (error) {
            if (error == boost::asio::error::eof) {
                std::cout << "Connection closed by server." << std::endl;
                return;
            }
            throw boost::system::system_error(error);
        }
    }

    std::istream response_stream(&response_buf);
    std::string header;
    std::getline(response_stream, header);
    if (!header.empty() && header.back() == '\r') {
        header.pop_back();
    }

    if (header.empty()) {
        std::cout << "(empty response)" << std::endl;
        return;
    }

    switch (header[0]) {

        // 성공 응답 처리
        case '+':
            std::cout << header.substr(1) << std::endl;
            break;

        // 에러 응답 처리
        case '-':
            std::cout << "(error) " << header.substr(1) << std::endl;
            break;

        // 문자열 응답 처리 (ping 등)
        case '$': {
            int str_len = std::stoi(header.substr(1));
            if (str_len == -1) {
                std::cout << "(nil)" << std::endl;
            } else {
                size_t to_read = str_len + 2; // for \r\n
                if (response_buf.size() < to_read) {
                    boost::asio::read(socket, response_buf, boost::asio::transfer_exactly(to_read - response_buf.size()), error);
                }
                if (error) throw boost::system::system_error(error);

                std::string bulk_str;
                bulk_str.resize(str_len);
                response_stream.read(&bulk_str[0], str_len);
                
                std::string dummy;
                std::getline(response_stream, dummy);

                std::cout << '"' << bulk_str << '"' << std::endl;
            }
            break;
        }

        // 정수형 응답 처리 (delete 등)
        case ':':
            std::cout << "(integer) " << header.substr(1) << std::endl;
            break;

        // 배열 응답 처리 (keys 등)
        case '*': {
            int array_len = std::stoi(header.substr(1));
            std::cout << array_len << " elements:" << std::endl;
            for (int i = 0; i < array_len; ++i) {
                handle_response_recursive(socket, response_buf);
            }
            break;
        }
        default:
            std::cout << "Unknown response type: " << header << std::endl;
            break;
    }
}
