# Mini-Redis
![License](https://img.shields.io/github/license/senli1073/senli1073.github.io)
![Last Commit](https://img.shields.io/github/last-commit/DongInSong/mini-redis)
## 1. Overview
C++와 Boost.Asio를 사용하여 Redis의 핵심 기능(GET, SET, PUB/SUB, TTL 등)을 구현하는 학습용 프로젝트입니다.   
A learning-oriented project that implements core Redis features (GET, SET, DEL, PUB/SUB, TTL, etc.) using C++ and Boost.Asio.

## 2. Tech Stack

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)  ![Boost](https://img.shields.io/badge/Boost-00599C?style=for-the-badge&logoColor=white)


## 3. Project Progress  ![progress](https://img.shields.io/badge/Progress-100%25-green)

### Milestone 1: TCP Server and PING Command

-   [x] **Asynchronous TCP Server**: Built a basic asynchronous TCP server using Boost.Asio to support multiple simultaneous client connections.
-   [x] **TCP Client Implementation**: Created a simple synchronous client to connect, send commands, and receive responses from the server.
-   [x] **RESP Parser**: Implemented a parser to decode RESP (REdis Serialization Protocol) array format commands.
-   [x] **Command Handler**: Developed a command handler structure to process parsed commands.
-   [x] **`PING` Command**: Implemented the `PING` command to validate the communication and command flow between the client and server.

### Milestone 2: Basic Data Operations

-   [x] **In-memory Data Store**: Implement an internal store to manage key-value data.
-   [x] **`SET` Command**: Implement storing a value for a specified key.
-   [x] **`GET` Command**: Implement retrieving the value of a specified key.
-   [x] **`DEL` Command**: Implement key deletion. Returns the number of keys that were removed.
-   [x] **`KEYS` Command**: Implement key searching with glob-style patterns (e.g., `KEYS *`, `KEYS user:*`).

### Milestone 3: Advanced Features
-   [x] **`PUB/SUB` Commands**: Implement publish/subscribe messaging functionality.
-   [x] **TTL (Time To Live)**: Implement expiration for keys.

### Milestone 4: Integration with RSS-Redis Project
-   [x] Apply to [![GitHub](https://img.shields.io/badge/rss_redis-181717?style=flat&logo=github&logoColor=white)](https://github.com/DongInSong/rss-redis) and compare performance with existing Redis.

## 4. Build & Execution

### Requirements
- C++17 compatible compiler (MSVC, GCC, Clang)
- CMake (3.10 or higher)
- vcpkg package manager

### Build Steps

1. **Install Dependencies with vcpkg**
   The project requires `boost`, `gtest` (for tests), and `yaml-cpp`.
    ```bash
    # From your vcpkg directory
    # For Windows
    ./vcpkg install boost:x64-windows gtest:x64-windows yaml-cpp:x64-windows

    # For Linux / macOS
    ./vcpkg install boost gtest yaml-cpp
    ```

2. **Clone the Repository**
    ```bash
    git clone https://github.com/DongInSong/mini-redis.git
    cd mini-redis
    ```

3. **Configure the Build with CMake**
   Create a build directory and run CMake. The project will automatically find the vcpkg toolchain if you have the `VCPKG_ROOT` environment variable set.
    ```bash
    mkdir build
    cd build
    cmake ..
    ```
   If you do not have the `VCPKG_ROOT` environment variable set, you must specify the path to the toolchain file manually:
    ```bash
    # Replace <path-to-vcpkg> with the actual path to your vcpkg installation
    cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
    ```

4. **Build the Project**
    ```bash
    cmake --build .
    ```
    Executables will be generated in the `build/Debug` (or `build/Release`) directory.

### Run

1. **(Optional) Modify Configuration**
   Edit `config.yaml` in the project root to change the server's host or port.
    ```yaml
    server:
      host: 0.0.0.0
      port: 6379
    ```

2. **Start the Server**
   The server executable must be run from the project root directory to find `config.yaml`.
    ```bash
    # From the 'build' directory
    ./Debug/mini-redis_server
    ```

3. **Start the Client**
   Open a new terminal to connect to the server.
    ```bash
    # From the 'build' directory
    ./Debug/client
    ```
    
## 5. Future Plans
-   [ ] Support for various data structures (List, Hash, Set, Sorted Set).
-   [ ] Implement advanced logging system (e.g., spdlog).
-   [ ] Implement persistence (RDB or AOF).

