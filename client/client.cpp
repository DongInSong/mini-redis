#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// 공유 상태
std::atomic<bool> g_subscribed_mode = false;

std::string parse_one_element(tcp::socket& socket, boost::asio::streambuf& response_buf);
void handle_response(tcp::socket& socket, boost::asio::streambuf& response_buf);


// 서버로부터 메시지를 읽어 처리하는 스레드 함수
void socket_reader(tcp::socket& socket) {
    try {
        while (true) {
            boost::asio::streambuf response_buf;
            handle_response(socket, response_buf);
        }
    } catch (std::exception& e) {
        if (std::string(e.what()).find("End of file") != std::string::npos) {
            std::cout << "\nConnection closed by server. Press Enter to exit." << std::endl;
        } else {
            std::cerr << "\nReader thread exception: " << e.what() << std::endl;
        }
    }
}

// 따옴표를 처리하는 명령어 파싱 함수
std::vector<std::string> parse_command_line(const std::string& line) {
    std::vector<std::string> parts;
    std::string current_part;
    bool in_quotes = false;
    char quote_char = '\0';

    for (char c : line) {
        if (!in_quotes && (c == '\'' || c == '"')) {
            in_quotes = true;
            quote_char = c;
            if (!current_part.empty()) {
                parts.push_back(current_part);
                current_part.clear();
            }
        } else if (in_quotes && c == quote_char) {
            in_quotes = false;
            parts.push_back(current_part);
            current_part.clear();
        } else if (!in_quotes && std::isspace(c)) {
            if (!current_part.empty()) {
                parts.push_back(current_part);
                current_part.clear();
            }
        } else {
            current_part += c;
        }
    }

    if (!current_part.empty()) {
        parts.push_back(current_part);
    }

    return parts;
}

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
        std::cout << "Example commands: PING, SET key value, GET key, SUBSCRIBE channel" << std::endl;
        
        // 리더 스레드 시작
        std::thread reader(socket_reader, std::ref(socket));
        reader.detach();

        for (std::string line; 
             std::cout << (g_subscribed_mode ? "(subscribe mode)> " : "> ") && std::getline(std::cin, line);) {
            if (line.empty()) continue;

            std::vector<std::string> cmd_parts = parse_command_line(line);

            if (cmd_parts.empty()) continue;

            std::string upper_cmd = cmd_parts[0];
            std::transform(upper_cmd.begin(), upper_cmd.end(), upper_cmd.begin(), ::toupper);

            if (g_subscribed_mode) {
                if (upper_cmd == "QUIT") {
                    // "QUIT" is an alias for "UNSUBSCRIBE" in sub mode to exit it.
                    cmd_parts = {"UNSUBSCRIBE"}; 
                    upper_cmd = "UNSUBSCRIBE";
                    // The mode will be set to false by the response handler
                }
                if (upper_cmd != "SUBSCRIBE" && upper_cmd != "UNSUBSCRIBE" && upper_cmd != "PING") {
                    std::cout << "(error) only SUBSCRIBE, UNSUBSCRIBE, PING, and QUIT are allowed in subscription mode." << std::endl;
                    continue;
                }
            }
            
            std::string request = encode_resp_command(cmd_parts);
            boost::asio::write(socket, boost::asio::buffer(request));
        }

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

void handle_response(tcp::socket& socket, boost::asio::streambuf& response_buf) {
    std::string element = parse_one_element(socket, response_buf);
    std::cout << element << std::endl;
}

std::string parse_one_element(tcp::socket& socket, boost::asio::streambuf& response_buf) {
    boost::system::error_code error;
    
    if (response_buf.size() == 0) {
        boost::asio::read_until(socket, response_buf, "\r\n", error);
        if (error) {
            if (error == boost::asio::error::eof) return "Connection closed by server.";
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
        return "(empty response)";
    }

    switch (header[0]) {
        case '+': // Simple String
            return header.substr(1);
        case '-': // Error
            return "(error) " + header.substr(1);
        case ':': // Integer
            return "(integer) " + header.substr(1);
        case '$': { // Bulk String
            int str_len = std::stoi(header.substr(1));
            if (str_len == -1) {
                return "(nil)";
            }
            size_t to_read = str_len + 2; // for \r\n
            if (response_buf.size() < to_read) {
                boost::asio::read(socket, response_buf, boost::asio::transfer_exactly(to_read - response_buf.size()), error);
            }
            if (error) throw boost::system::system_error(error);

            std::string bulk_str;
            bulk_str.resize(str_len);
            response_stream.read(&bulk_str[0], str_len);
            
            std::string dummy;
            std::getline(response_stream, dummy); // consume CRLF

            return '"' + bulk_str + '"';
        }
        case '*': { // Array
            int array_len = std::stoi(header.substr(1));
            std::string result = std::to_string(array_len) + " elements:\n";
            std::vector<std::string> elements;
            for (int i = 0; i < array_len; ++i) {
                elements.push_back(parse_one_element(socket, response_buf));
            }

            if (!elements.empty()) {
                if (elements[0] == "\"subscribe\"") {
                    g_subscribed_mode = true;
                } else if (elements[0] == "\"unsubscribe\"") {
                    // The third element is the remaining subscription count, sent as a bulk string.
                    if (elements.size() == 3 && elements[2] == "\"0\"") {
                        g_subscribed_mode = false;
                    }
                }
            }
            
            for (size_t i = 0; i < elements.size(); ++i) {
                result += std::to_string(i+1) + ") " + elements[i];
                if (i < elements.size() - 1) result += "\n";
            }
            return result;
        }
        default:
            return "Unknown response type: " + header;
    }
}
