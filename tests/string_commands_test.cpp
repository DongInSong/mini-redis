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
