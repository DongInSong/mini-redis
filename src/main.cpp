#include "network/server.hpp"
#include <iostream>

/* MINI-REDIS SERVER 
* @author @DongInSong
* @date 2025-07-12
* This is a simple implementation of a Redis-like server using C++ and Boost.Asio.
* 
* ## 개요
* mini-redis는 C++와 Boost.Asio를 사용하여 Redis 서버를 간단히 재현한 프로젝트입니다.
*
* ## 주요 기능
* - 문자열 데이터 타입 지원
* - pub/sub 기능
* - RESP 프로토콜 구현
* - 멀티스레드 I/O 서비스
* - 명령어 핸들러를 통한 명령어 처리
* - 명령어 파싱 및 직렬화
* - 키 만료 기능
* - 다양한 데이터 타입(LIST, HASH, SET, SORTED SET) 추가 가능한 확장성있는 구조
*
* ## 명령어 구현 방식
* store_: generic(PING, DEL, KEYS), string(GET, SET, SETEX, INCR, DECR, INCRBY, DECRBY) 직접적 구현
* pubsub_manager_: pub/sub(SUBSCRIBE, UNSUBSCRIBE, PUBLISH) 직접적 구현
* string_command_handler.cpp, generic_command_handler.cpp, pubsub_command_handler.cpp에 명령어 추가
* 
* ## 작동 순서
* 1. main.cpp에서 서버 시작
* 2. server.cpp에서 acceptor 생성 및 클라이언트 연결 수락 (cpu 코어 수 + 메인 스레드 풀 생성)
* 3. session.cpp에서 클라이언트와의 세션 생성
* 4. parser.cpp에서 클라이언트 요청 파싱
* 5. command/dispatcher.cpp에서 명령어 핸들러 선택
* 6. 각 핸들러는 command_handler_interface를 상속받아 execute() 메서드를 구현
* 7. session.cpp에서 핸들러를 사용하여 명령어 실행
* 8. 응답은 protocol/serializer.cpp에서 RESP 형식으로 직렬화됨
* 9. session.cpp에서 클라이언트로 전송
*
* ## 계획
* - [ ] store 구조 확장 (main-store, string-store, list-store 등)
* - [ ] LIST, HASH, SET, SORTED SET 등 데이터 타입 추가
*/

int main()
{
    try
    {
        mini_redis::server s(6379);
        std::cout << "Mini-Redis server started on port 6379" << std::endl;
        s.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}