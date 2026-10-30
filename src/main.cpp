#include "lob/SimpleOrderBook.hpp"
#include <iostream>

int main() {
    lob::SimpleOrderBook book;
    book.add_order({1, lob::Side::Sell, 100, 15});
    auto trades = book.add_order({2, lob::Side::Buy, 100, 10});

    std::cout << "Executed " << trades.size() << " trade(s). "
              << "Remaining best ask qty: " << book.asks().front().quantity << "\n";
    return 0;
}
