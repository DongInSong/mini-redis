# 2. 시스템 설계

[**&#8592; 목차로 돌아가기**](./00_README.md)

## 2.1. Architecture

- Mini-Redis 서버는 각 컴포넌트가 명확한 책임을 갖도록 모듈식으로 설계되었습니다. 클라이언트의 요청이 서버에 도달하여 처리되고 응답하기까지의 흐름은 다음과 같습니다.

```
[Client] -> [Network (Session)] -> [Protocol (Parser)] -> [Command (Dispatcher)] -> [Handlers] -> [Storage / PubSub Manager]
     ^                                                                                                         |
     |                                                                                                         |
     +---------------------------------- [Protocol (Serializer)] <---------------------------------------------+
```

1.  **Network (`session`)**
     > 클라이언트와의 TCP 연결을 관리하고 비동기적으로 데이터를 읽고 씁니다.
2.  **Protocol (`parser`, `serializer`)**
     > 클라이언트로부터 받은 RESP 형식의 원시 데이터를 서버가 이해할 수 있는 명령어 객체로 파싱하고, 처리 결과를 다시 RESP 형식으로 직렬화하여 클라이언트에게 보냅니다.
3.  **Command (`dispatcher`, `handlers`)**
     > 파싱된 명령어를 받아 어떤 로직으로 처리할지 결정하고, 해당 로직을 가진 핸들러(ex. `string_command_handler`)에게 작업을 위임합니다.
4.  **Storage / PubSub**
     > 실제 데이터의 저장, 조회, 수정, 삭제를 담당하는 `store`와 PUB/SUB 메시지 중개를 담당하는 `pubsub_manager`가 핵심 로직을 수행합니다.

## 2.2. 비동기 네트워크 모델 (Proactor 패턴)

-   **핵심 컴포넌트**: `boost::asio::io_context`
-   **패턴** : `proactor`
     > 모든 I/O 작업(연결 수락, 데이터 읽기 등)은 비동기적으로 시작되며, OS가 작업 완료를 알려주면 `io_context`가 미리 등록된 핸들러(콜백 함수)를 실행시키는 구조입니다.
-   **스레드 풀**
     > 서버 시작 시 CPU 코어 수와 동일한 개수의 스레드 풀을 생성합니다. 모든 스레드는 `io_context.run()` 루프를 실행하며, `io_context`의 작업 큐에 들어온 작업들을 경쟁적으로 가져가 처리합니다.
-   **장점**
     > "클라이언트당 스레드 하나"를 할당하는 비효율적인 방식을 피하고, 소수의 스레드로 수많은 동시 접속을 효율적으로 처리할 수 있어 높은 확장성을 가집니다.

## 2.3. 명령어 처리 (Strategy 패턴)

-   **문제점**
     > 하나의 `commands` 클래스에서 `if-else`문으로 명령어를 처리하였으나 String 명령어 구현 후 PUB/SUB 명령어를 구현하는 과정에서 조건문의 처리와 명령어 구현부가 너무 방대해져 새로운 명령어 추가에 많은 시간이 소요되었습니다.
-   **해결책**: 전략(Strategy) 디자인 패턴을 적용하여 문제를 해결했습니다.
    >-   `ICommandHandler`: 모든 핸들러가 구현해야 할 `supports(명령어)`와 `execute(인자)` 메소드를 정의하는 인터페이스입니다.
    >-   `StringCommandHandler`, `PubSubCommandHandler` 등: 특정 명령어 그룹(문자열, Pub/Sub 등)의 처리 책임을 갖는 구체적인 전략 클래스입니다.
    >-   `CommandDispatcher`: 사용 가능한 모든 전략 객체(핸들러)의 목록을 소유하는 컨텍스트 클래스입니다. 명령어를 받으면, 핸들러 목록을 순회하며 해당 명령어를 `supports`하는 핸들러를 찾아 실행을 위임합니다.
-   **장점**: 명령어 호출부와 실제 구현부를 분리하여, 기존 코드를 수정하지 않고 새로운 핸들러 클래스를 추가하는 것만으로 쉽게 새 기능을 확장할 수 있도록 합니다 (개방 폐쇄 원칙).

## 2.4. 데이터 저장소 설계

-   **핵심 컴포넌트**: `storage::store`
-   **자료구조**
     > `std::unordered_map<std::string, value_entry>`를 핵심 인메모리 Key-Value 저장소로 사용합니다. 해시맵을 사용하여 평균 O(1)의 빠른 조회 속도를 보장합니다.
-   **다양한 타입 지원**
     > `value_entry` 구조체 내에서 `std::variant<RedisString, RedisList, ...>`를 사용하여 값(value)을 저장합니다. 이는 향후 `LIST`, `HASH` 등 새로운 데이터 타입을 추가할 때, `store` 클래스의 기본 구조를 변경하지 않고도 유연하게 확장할 수 있는 기반을 만들었습니다.
-   **동시성 제어**
     > `store` 클래스 내의 모든 public 메소드는 `std::lock_guard<std::mutex>`를 사용하여, 멀티스레드 환경에서 여러 스레드가 동시에 데이터에 접근하더라도 데이터의 일관성과 무결성이 깨지지 않도록 보장합니다.

## 2.5. 메모리 관리 (스마트 포인터)

-   **과제**: 비동기 콜백 함수와 복잡하게 얽혀있는 `session` 객체의 생명주기를 안전하게 관리하는 것이 중요했습니다.
-   **해결책**: 모던 C++의 스마트 포인터 활용
    -   `std::shared_ptr`: `session` 객체는 `shared_ptr`로 관리하여, 참조하는 비동기 작업이 하나라도 남아있는 동안에는 객체가 소멸되지 않도록 보장합니다.
    -   `std::weak_ptr`: `session`과 `PubSubCommandHandler` 간의 순환 참조를 방지하기 위해 핸들러는 `session`을 `weak_ptr`로 참조합니다. 이는 소유권을 갖지 않는 '약한 참조'이므로, `session` 객체가 정상적으로 소멸될 수 있도록 하여 메모리 누수를 방지합니다.
    ```
         # command.dispatch.cpp
    
         * PUB/SUB 핸들러는 다른 핸들러와 다르게 명령 처리를 위해 세션이 필요함.
         * 따라서 Dispatcher가 핸들러에게 세션을 설정할 수 있도록 별도의 메서드를 제공.
         * session.cpp에서 세션 생성 후, handler_.set_session(weak_from_this())를 호출하여 세션을 설정함.
         * 
         * # What is shared_ptr and weak_ptr?
         * shared_ptr는 객체의 소유권을 공유하는 스마트 포인터로, 참조 카운트를 관리(추가되면 +1, 파괴되면 -1)하여 객체가 더 이상 사용되지 않을 때 자동으로 메모리를 해제함.
         * 그러나 만약 객체 A와 객체 B가 서로 shared_ptr로 참조하고 있다면, 이후 A와 B를 가리키던 다른 ptr이 모두 사라져도 두 객체가 서로를 계속 가르키고 있기 때문에 순환 참조가 발생할 수 있음.
         * weak_ptr는 shared_ptr의 약한 참조로, 객체의 소유권을 가지지 않으며, 참조 카운트에 영향을 주지 않음.
         * 
         * # Why waek_from_this()?
         * 순환 참조를 방지하기 위해 세션이 핸들러를 소유하지 않도록 하기 위함.
         * session -> CommandDispatcher -> PubSubCommandHandler 구조에서 shared_ptr를 사용하면 순환 참조가 발생할 수 있음. -> Cause memory leak
         * PUB/SUB 핸들러가 세션을 weak_ptr를 유지함으로써, 세션 생명 주기에 영향 x.
         * 따라서 외부에서 세션을 가리키는 마지막 shared_ptr가 사라지면 세션이 파괴되고, 파괴되면서 연쇄적으로 자신이 소유한 dispatcher와 핸들러도 파괴됨.
    ```

---
[**&#8592; 이전: 서론**](./01_Introduction.md) | [**다음: 핵심 기능 구현 &#8594;**](./03_Core_Features.md)