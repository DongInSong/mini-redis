# 3. 핵심 기능 구현

[**&#8592; 목차로 돌아가기**](./00_README.md)

## 3.1. RESP (REdis Serialization Protocol)

클라이언트–서버 간의 모든 통신은 Redis의 표준 프로토콜인 RESP(REdis Serialization Protocol)를 따릅니다. 이를 위해 프로토콜 파싱과 직렬화를 담당하는 로직을 독립된 모듈로 분리하여 설계했습니다.

-   **Parser (`protocol::parser`)**
    -   네트워크 버퍼로부터 읽어 들인 원시 바이트 스트림을 분석하여 의미있는 명령어 단위로 변환합니다. TCP 스트림 특성상 데이터가 나눠져 도착할 수 있기 때문에, 파서는 내부적으로 상태를 유지하며 여러 조각을 조합 및 재구성할 수 있도록 설계했습니다.
    -   `*`(Array), `$`(Bulk String) 등의 RESP 타입 프리픽스를 해석하고, 명시된 길이를 기반으로 데이터를 정확히 읽어들입니다.
    -   최종적으로 `std::vector<std::string>` 형태의 명령어 객체(`command_t`)로 반환됩니다.   
        ```
        *3\r\n$3\r\nSET\r\n$5\r\nhello\r\n$5\r\nworld\r\n
        → ["SET", "hello", "world"]
        ```

-   **Serializer (`protocol::serializer`)**
    -   서버의 처리 결과를 RESP 형식의 응답 문자열로 변환합니다. (ex. 성공 시에는 `+OK\r\n`, 정수 반환 시에는 `:1000\r\n`, 문자열 반환 시에는 `$5\r\nhello\r\n`)
    -   모든 직렬화 함수는 static 메서드로 구현되어 있어, 다른 모듈에서 쉽게 다음과 같이 호출할 수 있습니다:
          ```
          serializer::serialize_integer(100);
          serializer::serialize_simple_string("PONG");
          serializer::serialize_bulk_string("value");
          ```

-    **파싱 처리 예시**
        ```
        *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nhello\r\n
        ```
        1. `*3\r\n` -> 배열 길이
           - `pos = 0` 에서 `*` 확인
           - pos를 `*` 다음으로 이동하여 정수 확인 (3 = 배열 길이, 명령어(SET) KEY(key) VALUE(hello))
           - `\r\n`까지 이동 후 `pos = 4`
        2. `$3\r\n` -> 첫번째 항목 길이 (Bulk String)
           - `pos = 4` 에서 `$` 확인
           - pos를 `$` 다음으로 이동하여 정수 확인 (길이가 2자리 일 수도 있음 `pos = 5~6`)
           - `\r\n`까지 이동 후 `pos = 8`
        3. `SET\r\n` -> 실제 문자열 추출
           - `pos = 8` 에서 `3` 바이트 만큼 읽음 ("SET")
           - `pos = 11`, 이후 `\r\n` 확인 -> `pos = 13`
        4. 이후 `$` 기준으로 반복하여 key 값과 value 값을 파싱하여 `SET key hello` 명령어 실행

## 3.2. 주요 명령어 구현

-   **String Commands (`SETEX`, `INCRBY`)**:
    -   **`SETEX` 처리 흐름 (TTL 설정)**:
        1. 클라이언트로부터 바이트스트림 `*4\r\n$5\r\nSETEX\r\n$5\r\nmykey\r\n$2\r\n10\r\n$5\r\nhello\r\n`를 수신
        2. `parser`에서 `{"SETEX", "mykey", "10", "hello"}`로 파싱하여 `dispatcher`로 전달
        3. `dispatcher`에서 명령어를 구분하여 `string_command_handler`로 전달
        4. 벡터의 크기와 `SETEX`의 인자 수 4를 비교하여 검증 
        5. 성공 시, `store`에 `setex(key, ttl, value)` 호출
        6. `store`은 현재 시간을 기준으로 `std::chrono::steady_clock`을 사용해 만료 시간을 계산하여 데이터 저장
        7. 이후 클라이언트에 `serializer::serialize_ok()` 송신 (`+OK\r\n`)
           
    -   **`INCRBY` 처리 흐름**:
        1.  클라이언트로부터 바이트스트림 `*3\r\n$6\r\nINCRBY\r\n$9\r\nmycounter\r\n$2\r\n10\r\n`를 수신
        2.  `parser`에서 `{"INCRBY", "mycounter", "10"}`로 파싱하여 `dispatcher`로 전달
        3.  `dispatcher`에서 명령어를 구분하여 `string_command_handler`로 전달
        4.  벡터의 크기와 `INCRBY`의 인자 수 3를 비교하여 검증
        5.  성공 시, `store`에 `incrby("mycounter", 10)` 호출
        6.  `store`은 `data_.find`로 키를 찾거나 생성하여 `10`을 더하여 최종 값 반환
        7.  이후 클라이언트에 `serializer::serialize_integer(10)` 송신 (`:10\r\n`) 

-   **Pub/Sub Commands (`SUBSCRIBE`, `PUBLISH`)**:
    -   `pubsub_manager`는 PUB/SUB 기능의 중재자 역할을 하며, 클라이언트 세션들과 채널을 연결합니다.
    -   내부적으로 다음과 같은 자료구조를 사용
        ```
        std::unordered_map<std::string, std::list<std::weak_ptr<session>>>
        ```
    -   각 채널(`std::string`)에 대해 구독 중인 세션 목록을 관리합니다.
    -   `weak_ptr`를 사용하여 `session` 소멸 시 안전하게 자동으로 구독 목록에서 제거될 수 있도록 합니다.
      
    -   **`SUBSCRIBE` 흐름**
        1. 클라이언트가` SUBSCRIBE news`를 요청 (명령어 파싱 과정은 위 String commands와 동일)
        2. `pubsub_command_handler`가 `pubsub_manager`에 `subscribe("news", session_ptr)` 호출
        3. `pubsub_manager`는 해당 채널 구독자 리스트에 `session_ptr`을 `weak_ptr` 형태로 추가
        4. 이후 클라이언트는 `subscribe mode`에 들어가며 채널에 메시지가 발행되면 자동으로 수신
           
    -   **`PUBLISH` 흐름**
        1. 클라이언트가 `PUBLISH news "THIS IS NEWS"`를 요청
        2. `pubsub_command_handler`가 `pubsub_manager`에 `publish("news", message)` 호출
        3. `pubsub_manager`는 `news` 채널의 구독자 리스트를 순회하며 `weak_ptr`를 `lock()`하여 살아있는 세션 필터링
        4. 유효한 세션은 `session`에 `deliver(message)` 호출
        5. 메시지는 비동기적으로 구독자에게 전송 (Boost.Asio의 async_write와 큐를 사용)

-   **키 만료 (`TTL`)**:
    - Lazy Expiration
        - 키의 만료는 접근 시점에 확인합니다.
          1. 클라이언트가 `GET key`를 요청
          2. 처리 전 `is_key_expired()`를 통해 현재 시간과 `value_entry.expiry_time`을 비교
          3. 만료 시 `serializer::serialize_null_bulk_string()` 송신 (`nil`)

    - Semi-Active Expiration
        - `KEYS` 명령으로 전체 키를 순회할 때 만료된 키를 정리

## 3.3. 설정 관리 (YAML)
-   **`Config` 클래스**
    > 설정 파일(config.yaml)을 읽고 파싱하는 모든 로직은 별도의 Config 클래스로 캡슐화했습니다.
    > 파일이 존재하지 않거나 포맷이 잘못된 경우를 대비해, 예외 처리도 클래스 내부에 포함하여 안정성을 강화했습니다.
    ``` yaml
    server:
      host: 127.0.0.1
      port: 6379
    ```
-   **의존성 분리**
    > `main` 함수가 `Config` 객체를 생성하여 여기서 얻은 설정 값(호스트, 포트)을 `server` 생성자에 인자로 전달합니다.

---
[**&#8592; 이전: 시스템 설계**](./02_System_Design.md) | [**다음: 테스트 및 검증 &#8594;**](./04_Testing.md)