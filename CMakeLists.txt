cmake_minimum_required(VERSION 3.10)

project(mini-redis CXX)
cmake_policy(SET CMP0167 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Boost 라이브러리 찾기 (system 및 asio)
find_package(Boost REQUIRED COMPONENTS asio)

# 소스 파일 목록
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 실행 파일 생성
add_executable(mini-redis_server ${SOURCES})

# 헤더 파일 경로 추가 (modern cmake)
target_include_directories(mini-redis_server PUBLIC include)

# 라이브러리 링크
target_link_libraries(mini-redis_server PRIVATE Boost::asio)

# 테스트용 클라이언트 빌드
add_executable(client client/client.cpp)
target_include_directories(client PUBLIC include)

# 테스트 코드 빌드 (gtest 같은 프레임워크 추가 필요)
# enable_testing()
# file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
# add_executable(run_tests ${TEST_SOURCES})
# target_link_libraries(run_tests gtest gtest_main)
# add_test(NAME unit_tests COMMAND run_tests)
