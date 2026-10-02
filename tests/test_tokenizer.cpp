#include "parser/Tokenizer.hpp"

#include <gtest/gtest.h>

using Parser::Tokenizer;

TEST(Tokenizer, EmptyLine) {
    Tokenizer t;
    EXPECT_TRUE(t("").empty());
    EXPECT_TRUE(t("   ").empty());
    EXPECT_TRUE(t("\t\r\n").empty());
}

TEST(Tokenizer, BasicSplit) {
    Tokenizer t;
    auto v = t("SET key value");
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "SET");
    EXPECT_EQ(v[1], "key");
    EXPECT_EQ(v[2], "value");
}

TEST(Tokenizer, CollapsesWhitespace) {
    Tokenizer t;
    auto v = t("  SET\t\tkey\n\rvalue  ");
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "SET");
    EXPECT_EQ(v[1], "key");
    EXPECT_EQ(v[2], "value");
}