#include "command/dispatcher.hpp"
#include "command/generic_command_handler.hpp"
#include "command/string_command_handler.hpp"
#include "command/pubsub_command_handler.hpp"
#include "protocol/serializer.hpp"
#include <algorithm>
#include <cctype>

namespace mini_redis
{
    CommandDispatcher::CommandDispatcher(std::shared_ptr<store> store, std::shared_ptr<pubsub_manager> pubsub_manager) {
        // unique_ptr를 사용하여 각 핸들러의 소유권을 명확히 하고, 핸들러가 더 이상 필요하지 않을 때 자동으로 메모리를 해제함.
        handlers_.push_back(std::make_unique<GenericCommandHandler>(store));
        handlers_.push_back(std::make_unique<StringCommandHandler>(store));
        
        /*
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
         */

        auto pubsub_handler = std::make_unique<PubSubCommandHandler>(pubsub_manager);
        // unique_ptr이라 복사 불가, std::move()로 handler_ 소유권 이전
        handlers_.push_back(std::move(pubsub_handler));
    }

    void CommandDispatcher::set_session(std::weak_ptr<session> s) {
        for (auto& handler : handlers_) {
            if (auto pubsub_handler = dynamic_cast<PubSubCommandHandler*>(handler.get())) {
                pubsub_handler->set_session(s);
            }
        }
    }

    std::string CommandDispatcher::execute_command(const command_t &cmd)
    {
        if (cmd.empty())
        {
            return serializer::serialize_error("ERR wrong number of arguments for 'empty' command");
        }

    std::string command_name = cmd[0];
 
    // 명령어 모두 대문자로 변환 get -> GET
    std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);

    for (const auto& handler : handlers_) {
            if (handler->supports(command_name)) {
                return handler->execute(cmd);
            }
        }

        return serializer::serialize_error("ERR unknown command `" + cmd[0] + "`");
    }
} // namespace mini_redis
