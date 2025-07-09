# Mini-Redis

C++와 Boost.Asio를 사용하여 Redis의 핵심 기능(GET, SET, PUB/SUB, TTL 등)을 구현하는 학습용 프로젝트입니다.

## 기술 스택

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)  ![Boost](https://img.shields.io/badge/Boost-00599C?style=for-the-badge&logoColor=white)

## 빌드 및 실행 방법

### 요구사항

-   C++17을 지원하는 컴파일러 (MSVC, GCC, Clang 등)
-   CMake (3.10 이상)
-   vcpkg

### 빌드

1.  **Boost 라이브러리 설치 (vcpkg)**
    ```bash
    vcpkg install boost:x64-windows
    ```

2.  **프로젝트 클론 및 `build` 디렉토리 생성**
    ```bash
    git clone https://github.com/DongInSong/mini-redis.git
    cd mini-redis
    mkdir build
    cd build
    ```

3.  **빌드 파일 생성**
    `vcpkg.cmake` 툴체인 파일의 경로 지정 필요!
    ```bash
    cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    ```

4.  **프로젝트 빌드**
    ```bash
    cmake --build .
    ```
    `build/Debug` 디렉토리에 `mini-redis_server.exe`와 `client.exe` 실행 파일 생성

### 실행

1.  **서버 실행**
    ```bash
    ./Debug/mini-redis_server.exe
    ```

2.  **클라이언트 실행**
    ```bash
    ./Debug/client.exe
    ```

## 진행 상황 

### Milestone 1: TCP 서버 및 PING 명령어

-   [x] **비동기 TCP 서버 구축**: Boost.Asio를 사용하여 여러 클라이언트의 동시 접속을 처리할 수 있는 비동기 TCP 서버의 기본 구조를 완성했습니다.
-   [x] **TCP 클라이언트 구현**: 서버에 접속하여 명령어를 전송하고 응답을 받을 수 있는 간단한 동기 방식의 클라이언트를 구현했습니다.
-   [x] **RESP 파서**: Redis 명령어 형식인 RESP의 배열 타입(*)을 해석할 수 있는 파서를 구현했습니다.
-   [x] **커맨드 핸들러**: 파싱된 명령어를 받아 처리하는 핸들러 구조를 만들었습니다.
-   [x] **`PING` 명령어 구현**: `PING`을 구현하여 서버와 클라이언트 간의 기본적인 통신 및 명령어 처리 흐름을 완성했습니다.

### Milestone 2: 기본 데이터 조작

-   [ ] **`SET` 명령어 구현**: 키(Key)에 값(Value)을 저장하는 기능을 추가합니다.
-   [ ] **`GET` 명령어 구현**: 키를 이용해 저장된 값을 조회하는 기능을 추가합니다.
-   [ ] **인메모리 저장소(`store`) 구현**
-   [ ] **`PUB/SUB` 명령어 구현**
-   [ ] **TTL(Time To Live) 구현**

### Milestone 3: Rss-Redis 프로젝트 적용
-   [ ] [![GitHub](https://img.shields.io/badge/rss_redis-181717?style=flat&logo=github&logoColor=white)](https://github.com/DongInSong/rss-redis) 적용 및 기존 Redis 성능 비교
