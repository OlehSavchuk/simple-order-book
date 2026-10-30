#include "lob/SimpleOrderBook.hpp"
#include <gtest/gtest.h>

using namespace lob;

TEST(OrderBookTest, RestingOrdersMaintainSpread) {
    SimpleOrderBook book;
    EXPECT_TRUE(book.add_order({1, Side::Buy, 99, 10}).empty());
    EXPECT_TRUE(book.add_order({2, Side::Sell, 101, 10}).empty());

    EXPECT_EQ(book.best_bid(), 99);
    EXPECT_EQ(book.best_ask(), 101);
}

TEST(OrderBookTest, PriceTimePriorityAndPartialFill) {
    SimpleOrderBook book;
    book.add_order({1, Side::Sell, 100, 10});
    book.add_order({2, Side::Sell, 100, 20});
    book.add_order({3, Side::Sell, 99,  5});

    auto trades = book.add_order({4, Side::Buy, 100, 20});

    std::vector<Trade> expected = {
        {.resting_order_id = 3, .aggressor_order_id = 4, .price = 99,  .quantity = 5},
        {.resting_order_id = 1, .aggressor_order_id = 4, .price = 100, .quantity = 10},
        {.resting_order_id = 2, .aggressor_order_id = 4, .price = 100, .quantity = 5}
    };
    EXPECT_EQ(trades, expected);

    ASSERT_EQ(book.asks().size(), 1u);
    EXPECT_EQ(book.asks().front().id, 2u);
    EXPECT_EQ(book.asks().front().quantity, 15u);
}

TEST(OrderBookTest, CancelRemovesOrder) {
    SimpleOrderBook book;
    book.add_order({10, Side::Buy, 50, 100});
    EXPECT_TRUE(book.cancel_order(10));
    EXPECT_FALSE(book.cancel_order(10));
    EXPECT_FALSE(book.best_bid().has_value());
}
