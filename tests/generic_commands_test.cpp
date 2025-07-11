#include "gtest/gtest.h"
#include "storage/store.hpp"
#include <thread>
#include <chrono>
#include <algorithm>

class GenericCommandsTest : public ::testing::Test {
protected:
    mini_redis::store store_instance;
};

/*
* Generic commands tests for the mini-redis store.
* These tests cover the basic functionality of the generic commands
* such as DEL, KEYS, EXISTS, EXPIRE, and TTL.
*/

TEST_F(GenericCommandsTest, Delete) {
    store_instance.set("key2", "value2");
    int result = store_instance.del("key2");
    EXPECT_EQ(result, 1);
    auto val = store_instance.get("key2");
    EXPECT_FALSE(val.has_value());
}

TEST_F(GenericCommandsTest, DeleteNonExistent) {
    int result = store_instance.del("non_existent_key");
    EXPECT_EQ(result, 0);
}

TEST_F(GenericCommandsTest, DeleteMultipleKeys) {
    store_instance.set("key1", "value1");
    store_instance.set("key2", "value2");
    store_instance.set("key3", "value3");
    int result = store_instance.del({"key1", "key3", "non_existent_key"});
    EXPECT_EQ(result, 2);
    EXPECT_FALSE(store_instance.get("key1").has_value());
    EXPECT_TRUE(store_instance.get("key2").has_value());
    EXPECT_FALSE(store_instance.get("key3").has_value());
}

TEST_F(GenericCommandsTest, KeysWildcardStar)
{
    store_instance.set("user:123", "Alice");
    store_instance.set("user:456", "Bob");
    store_instance.set("admin:789", "Eve");

    auto keys = store_instance.keys("user:*");

    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "user:123") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "user:456") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "admin:789") == keys.end());
}

TEST_F(GenericCommandsTest, KeysWildcardQuestionMark)
{
    store_instance.set("file1.txt", "data1");
    store_instance.set("file2.txt", "data2");
    store_instance.set("file10.txt", "data10");

    auto keys = store_instance.keys("file?.txt");

    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "file1.txt") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "file2.txt") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "file10.txt") == keys.end());
}

TEST_F(GenericCommandsTest, ExpireCommand) {
    store_instance.set("mykey", "myvalue");
    EXPECT_EQ(store_instance.ttl("mykey"), -1);
    store_instance.expire("mykey", 20);
    EXPECT_GT(store_instance.ttl("mykey"), 0);
    EXPECT_LE(store_instance.ttl("mykey"), 20);
}

TEST_F(GenericCommandsTest, KeysWithExpiration) {
    store_instance.set("key1", "value1");
    store_instance.setex("key2_exp", 1, "value2");
    
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto keys = store_instance.keys("*");
    EXPECT_EQ(keys.size(), 1);
    EXPECT_EQ(keys[0], "key1");
}
