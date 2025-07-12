cmake_minimum_required(VERSION 3.10)

project(mini-redis CXX)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
  include(${CMAKE_TOOLCHAIN_FILE})
elseif(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  include(${CMAKE_TOOLCHAIN_FILE})
else()
  message(FATAL_ERROR "vcpkg toolchain file not found. Please set CMAKE_TOOLCHAIN_FILE or VCPKG_ROOT environment variable.")
endif()

cmake_policy(SET CMP0167 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 라이브러리 찾기
find_package(Boost REQUIRED COMPONENTS asio)
find_package(GTest CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# 소스 파일 목록
file(GLOB_RECURSE ALL_SOURCES "src/**/*.cpp")
file(GLOB_RECURSE MAIN_SOURCE "src/main.cpp")
list(REMOVE_ITEM ALL_SOURCES ${MAIN_SOURCE})

set(SOURCES ${MAIN_SOURCE} ${ALL_SOURCES})
set(TEST_LIB ${ALL_SOURCES})

file(GLOB_RECURSE TEST_SOURCES
    "tests/*_test.cpp"
)

# 서버 빌드
add_executable(mini-redis_server ${SOURCES})
target_include_directories(mini-redis_server PUBLIC include)
target_link_libraries(mini-redis_server PRIVATE Boost::asio yaml-cpp::yaml-cpp)

# 테스트용 클라이언트 빌드
add_executable(client client/client.cpp)
target_include_directories(client PUBLIC include)
target_link_libraries(client PRIVATE Boost::asio)

# 테스트 코드 추가
enable_testing()

# 테스트용 라이브러리 정의
add_library(test_lib ${TEST_LIB})
target_include_directories(test_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_lib PRIVATE Boost::asio)

# 테스트 실행 파일 빌드
add_executable(unit_tests ${TEST_SOURCES})
target_include_directories(unit_tests PUBLIC include)
target_link_libraries(unit_tests PRIVATE test_lib GTest::gtest GTest::gtest_main Boost::asio)

add_test(NAME unit_tests COMMAND $<TARGET_FILE:unit_tests>)
