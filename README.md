# Mini-Redis
![License](https://img.shields.io/github/license/senli1073/senli1073.github.io)
![Last Commit](https://img.shields.io/github/last-commit/DongInSong/mini-redis)
## 1. Overview
C++와 Boost.Asio를 사용하여 Redis의 핵심 기능(GET, SET, PUB/SUB, TTL 등)을 구현하는 학습용 프로젝트입니다.   
A learning-oriented project that implements core Redis features (GET, SET, DEL, PUB/SUB, TTL, etc.) using C++ and Boost.Asio.

## 2. Tech Stack

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)  ![Boost](https://img.shields.io/badge/Boost-00599C?style=for-the-badge&logoColor=white)


## 3. Project Progress  ![progress](https://img.shields.io/badge/Progress-70%25-yellowgreen)

### Milestone 1: TCP Server and PING Command

-   [x] **Asynchronous TCP Server**: Built a basic asynchronous TCP server using Boost.Asio to support multiple simultaneous client connections.
-   [x] **TCP Client Implementation**: Created a simple synchronous client to connect, send commands, and receive responses from the server.
-   [x] **RESP Parser**: Implemented a parser to decode RESP (REdis Serialization Protocol) array format commands.
-   [x] **Command Handler**: Developed a command handler structure to process parsed commands.
-   [x] **`PING` Command**: Implemented the `PING` command to validate the communication and command flow between the client and server.

### Milestone 2: Basic Data Operations

-   [x] **`SET` Command**: Implement storing a value for a specified key.
-   [x] **`GET` Command**: Implement retrieving the value of a specified key.
-   [x] **`DEL` Command**: Implement key deletion. Returns the number of keys that were removed.
-   [x] **`KEYS` Command**: Implement key searching with glob-style patterns (e.g., `KEYS *`, `KEYS user:*`).
-   [x] **In-memory Data Store**: Implement an internal store to manage key-value data.

### Milestone 3: Advanced Features
-   [x] **`PUB/SUB` Commands**: Implement publish/subscribe messaging functionality.
-   [x] **TTL (Time To Live)**: Implement expiration for keys.

### Milestone 4: Integration with RSS-Redis Project
-   [ ] Apply to [![GitHub](https://img.shields.io/badge/rss_redis-181717?style=flat&logo=github&logoColor=white)](https://github.com/DongInSong/rss-redis) and compare performance with existing Redis.
-   [ ] Support for various data structures (List, Hash, Set, Sorted Set).

## 4. Build & Execution

### Requirements
- A C++17-compatible compiler (MSVC, GCC, Clang, etc.)
- CMake (version 3.10 or higher)
- vcpkg package manager

### Build Steps

1. **Install Boost via vcpkg**
    ```bash
    # Windows
    vcpkg install boost:x64-windows
    
    # Linux
    ./vcpkg install boost
    
    # macOS
    xcode-select --install
    ```

2. **Clone the repository and create a build directory**
    ```bash
    git clone https://github.com/DongInSong/mini-redis.git
    cd mini-redis
    mkdir build
    cd build
    ```

3. **Generate build files**
    ```bash
    cmake ..
    ```
    Or specify a build tool and the `vcpkg.cmake` toolchain file:
    ```bash
    # Windows
    cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

    # Linux
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake

    # macOS
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
    ```

4. **Build the project**
    ```bash
    cmake --build .
    ```
    Executables `mini-redis_server.exe` and `client.exe` will be generated in the `build/Debug` directory.


### Run

1. **Start the server**
    ```bash
    ./Debug/mini-redis_server.exe
    ```

2. **Start the client**
    ```bash
    ./Debug/client.exe
    ```
