#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <iomanip>
#include "config.h"

class Order {
private:
    OrderId orderId;
    UserId userId;
    std::string timestamp;
    std::vector<CartItem> items;
    double totalAmount;
    std::string status;
    static OrderId nextId;  

public:
    Order() : orderId(0), userId(0), totalAmount(0.0), status("Pending") {}
    
    Order(UserId userId, const std::vector<CartItem>& items, double total)
        : userId(userId), items(items), totalAmount(total), status("Pending") {
        orderId = nextId++;
        updateTimestamp();
    }

    void updateTimestamp() {
        std::time_t now = std::time(nullptr);
        char buf[DATE_STR_LEN];
        std::strftime(buf, DATE_STR_LEN, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        timestamp = buf;
    }

    OrderId getId() const { return orderId; }
    static void setNextId(OrderId id) { nextId = id; }
    static OrderId getNextId() { return nextId; }
    
    UserId getUserId() const { return userId; }
    std::string getTimestamp() const { return timestamp; }
    const std::vector<CartItem>& getItems() const { return items; }
    double getTotal() const { return totalAmount; }
    std::string getStatus() const { return status; }
    void setStatus(const std::string& newStatus) { status = newStatus; }
    void setId(OrderId id) { orderId = id; }

    void writeToStream(std::ostream& os) const {
        os.write(reinterpret_cast<const char*>(&orderId), sizeof(orderId));
        os.write(reinterpret_cast<const char*>(&userId), sizeof(userId));
        
        size_t len = timestamp.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(timestamp.c_str(), len);
        
        len = status.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(status.c_str(), len);
        
        os.write(reinterpret_cast<const char*>(&totalAmount), sizeof(totalAmount));
        
        size_t itemCount = items.size();
        os.write(reinterpret_cast<const char*>(&itemCount), sizeof(itemCount));
        for (const auto& item : items) {
            os.write(reinterpret_cast<const char*>(&item.productId), sizeof(item.productId));
            os.write(reinterpret_cast<const char*>(&item.quantity), sizeof(item.quantity));
        }
    }

    static Order readFromStream(std::istream& is) {
        Order order;
        
        is.read(reinterpret_cast<char*>(&order.orderId), sizeof(order.orderId));
        is.read(reinterpret_cast<char*>(&order.userId), sizeof(order.userId));
        
        size_t len;
        is.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 0 && len < 1000) {
            order.timestamp.resize(len);
            is.read(&order.timestamp[0], len);
        }
        
        is.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 0 && len < 100) {
            order.status.resize(len);
            is.read(&order.status[0], len);
        }
        
        is.read(reinterpret_cast<char*>(&order.totalAmount), sizeof(order.totalAmount));
        
        size_t itemCount;
        is.read(reinterpret_cast<char*>(&itemCount), sizeof(itemCount));
        if (itemCount <= MAX_CART_ITEMS) {
            order.items.resize(itemCount);
            for (size_t i = 0; i < itemCount; ++i) {
                is.read(reinterpret_cast<char*>(&order.items[i].productId), sizeof(order.items[i].productId));
                is.read(reinterpret_cast<char*>(&order.items[i].quantity), sizeof(order.items[i].quantity));
            }
        }
        
        return order;
    }

    void display(const std::vector<Product>& products) const {
        std::cout << "\n=== ORDER #" << orderId << " ===\n";
        std::cout << "Date: " << timestamp << " | Status: " << status << "\n";
        std::cout << "Items:\n";
        
        for (const auto& item : items) {
            auto productIt = std::find_if(products.begin(), products.end(),
                [item](const Product& p) { return p.getId() == item.productId; });
            
            if (productIt != products.end()) {
                std::cout << "  - " << productIt->getName() << " x" << item.quantity 
                          << " @ $" << productIt->getPrice() << "\n";
            } else {
                std::cout << "  - [Product ID " << item.productId << " no longer available] x" << item.quantity << "\n";
            }
        }
        
        printf("Total: $%.2f\n", totalAmount);
    }
};

OrderId Order::nextId = 1;
