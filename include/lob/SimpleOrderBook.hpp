#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

namespace lob {

enum class Side { Buy, Sell };

struct Order {
    uint64_t id;
    Side side;
    int64_t price;
    uint64_t quantity;
};

struct Trade {
    uint64_t resting_order_id;
    uint64_t aggressor_order_id;
    int64_t price;
    uint64_t quantity;

    bool operator==(const Trade&) const = default; // C++20 default equality for clean test assertions
};

class SimpleOrderBook {
public:
    std::vector<Trade> add_order(Order incoming) {
        std::vector<Trade> trades;

        if (incoming.side == Side::Buy) {
            match(incoming, asks_, trades, [](int64_t buy_p, int64_t ask_p) {
                return buy_p >= ask_p;
            });
            if (incoming.quantity > 0) {
                auto it = std::upper_bound(bids_.begin(), bids_.end(), incoming.price,
                    [](int64_t price, const Order& existing) {
                        return price > existing.price;
                    });
                bids_.insert(it, incoming);
            }
        } else {
            match(incoming, bids_, trades, [](int64_t sell_p, int64_t bid_p) {
                return sell_p <= bid_p;
            });
            if (incoming.quantity > 0) {
                auto it = std::upper_bound(asks_.begin(), asks_.end(), incoming.price,
                    [](int64_t price, const Order& existing) {
                        return price < existing.price;
                    });
                asks_.insert(it, incoming);
            }
        }
        return trades;
    }

    bool cancel_order(uint64_t order_id) {
        if (std::erase_if(bids_, [order_id](const Order& o) { return o.id == order_id; }) > 0) {
            return true;
        }
        return std::erase_if(asks_, [order_id](const Order& o) { return o.id == order_id; }) > 0;
    }

    std::optional<int64_t> best_bid() const {
        return bids_.empty() ? std::nullopt : std::make_optional(bids_.front().price);
    }

    std::optional<int64_t> best_ask() const {
        return asks_.empty() ? std::nullopt : std::make_optional(asks_.front().price);
    }

    const std::vector<Order>& bids() const { return bids_; }
    const std::vector<Order>& asks() const { return asks_; }

private:
    std::vector<Order> bids_;
    std::vector<Order> asks_;

    template <typename CrossPredicate>
    void match(Order& incoming, std::vector<Order>& opposite_book,
               std::vector<Trade>& trades, CrossPredicate can_cross) {
        while (incoming.quantity > 0 && !opposite_book.empty()) {
            Order& best_resting = opposite_book.front();
            if (!can_cross(incoming.price, best_resting.price)) {
                break;
            }

            uint64_t fill_qty = std::min(incoming.quantity, best_resting.quantity);
            trades.push_back(Trade{
                .resting_order_id = best_resting.id,
                .aggressor_order_id = incoming.id,
                .price = best_resting.price,
                .quantity = fill_qty
            });

            incoming.quantity -= fill_qty;
            best_resting.quantity -= fill_qty;

            if (best_resting.quantity == 0) {
                opposite_book.erase(opposite_book.begin());
            }
        }
    }
};

} // namespace lob
