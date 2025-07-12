#include "gtest/gtest.h"
#include "storage/store.hpp"
#include <thread>
#include <chrono>

class StringCommandsTest : public ::testing::Test {
protected:
    mini_redis::store store_instance;
};

/*
* String commands tests for the mini-redis store.
* These tests cover the basic functionality of string commands
* such as SET, GET, SETEX.
*/

TEST_F(StringCommandsTest, SetAndGet) {
    store_instance.set("key1", "value1");
    auto val = store_instance.get("key1");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "value1");
    auto val2 = store_instance.get("key2");
    EXPECT_EQ(val2, std::nullopt);
}

TEST_F(StringCommandsTest, GetNonExistent) {
    auto val = store_instance.get("non_existent_key");
    EXPECT_FALSE(val.has_value());
}

TEST_F(StringCommandsTest, SetexAndGet) {
    store_instance.setex("key_ttl", 2, "value_ttl");
    auto val = store_instance.get("key_ttl");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "value_ttl");
}

TEST_F(StringCommandsTest, SetexExpiration) {
    store_instance.setex("key_exp", 1, "value_exp");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto val = store_instance.get("key_exp");
    EXPECT_FALSE(val.has_value());
}

TEST_F(StringCommandsTest, TTLCommand) {
    store_instance.set("key_no_ttl", "value");
    store_instance.setex("key_with_ttl", 10, "value");

    EXPECT_EQ(store_instance.ttl("key_no_ttl"), -1);
    EXPECT_GT(store_instance.ttl("key_with_ttl"), 0);
    EXPECT_LE(store_instance.ttl("key_with_ttl"), 10);
    EXPECT_EQ(store_instance.ttl("non_existent_key"), -2);
}

TEST_F(StringCommandsTest, Incr) {
    // 1. 키가 없을 때 INCR을 호출하면 1이 되어야 함
    EXPECT_EQ(store_instance.incr("counter"), 1);

    // 2. 다시 호출하면 2가 되어야 함
    EXPECT_EQ(store_instance.incr("counter"), 2);

    // 3. 다른 키에 대해서도 잘 동작하는지 확인
    EXPECT_EQ(store_instance.incr("other_counter"), 1);

    // 4. 정수가 아닌 값을 가진 키에 INCR을 시도하면 예외 발생
    store_instance.set("not_a_number", "hello");
    EXPECT_THROW(store_instance.incr("not_a_number"), std::runtime_error);
}

TEST_F(StringCommandsTest, Decr) {
    // 1. 키가 없을 때 DECR을 호출하면 -1이 되어야 함
    EXPECT_EQ(store_instance.decr("counter"), -1);

    // 2. 다시 호출하면 -2가 되어야 함
    EXPECT_EQ(store_instance.decr("counter"), -2);

    // 3. 다른 키에 대해서도 잘 동작하는지 확인
    store_instance.set("other_counter", "10");
    EXPECT_EQ(store_instance.decr("other_counter"), 9);

    // 4. 정수가 아닌 값을 가진 키에 DECR을 시도하면 예외 발생
    store_instance.set("not_a_number", "world");
    EXPECT_THROW(store_instance.decr("not_a_number"), std::runtime_error);
}

TEST_F(StringCommandsTest, IncrBy) {
    // 1. 키가 없을 때 INCRBY 10을 호출하면 10이 되어야 함
    EXPECT_EQ(store_instance.incrby("counter", 10), 10);

    // 2. 다시 5를 더하면 15가 되어야 함
    EXPECT_EQ(store_instance.incrby("counter", 5), 15);

    // 3. 음수를 더하면 뺄셈처럼 동작해야 함
    EXPECT_EQ(store_instance.incrby("counter", -5), 10);

    // 4. 정수가 아닌 값을 가진 키에 INCRBY를 시도하면 예외 발생
    store_instance.set("not_a_number", "hello");
    EXPECT_THROW(store_instance.incrby("not_a_number", 5), std::runtime_error);

    // 5. 잘못된 증감값에 대해 예외 발생 (이 테스트는 핸들러 레벨에서 필요)
}

TEST_F(StringCommandsTest, DecrBy) {
    // 1. 키가 없을 때 DECRBY 5를 호출하면 -5가 되어야 함
    EXPECT_EQ(store_instance.decrby("counter", 5), -5);

    // 2. 다시 5를 빼면 -10이 되어야 함
    EXPECT_EQ(store_instance.decrby("counter", 5), -10);

    // 3. 음수를 빼면 덧셈처럼 동작해야 함
    EXPECT_EQ(store_instance.decrby("counter", -5), -5);

    // 4. 정수가 아닌 값을 가진 키에 DECRBY를 시도하면 예외 발생
    store_instance.set("not_a_number", "world");
    EXPECT_THROW(store_instance.decrby("not_a_number", 5), std::runtime_error);
}
