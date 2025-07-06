cmake_minimum_required(VERSION 3.10)

project(mini-redis CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 파일 경로 추가
include_directories(include)

# 소스 파일 목록
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 실행 파일 생성
add_executable(mini-redis_server ${SOURCES})

# 테스트용 클라이언트 빌드
add_executable(simple_client client/simple_client.cpp)

# 테스트 코드 빌드 (gtest 같은 프레임워크 추가 필요)
# enable_testing()
# file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
# add_executable(run_tests ${TEST_SOURCES})
# target_link_libraries(run_tests gtest gtest_main)
# add_test(NAME unit_tests COMMAND run_tests)
