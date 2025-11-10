#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "config.h"

class User 
{
private:
    UserId id;
    std::string username;
    std::string passwordHash;
    UserType type;
    std::vector<OrderId> orderHistory;

public:
    User() : id(0), username("guest"), type(UserType::CUSTOMER) {}
    
    User(UserId id, const std::string& username, const std::string& password, UserType type) : id(id), username(username), type(type) 
    {
        passwordHash = hashPassword(password);
    }

    bool authenticate(const std::string& password) const 
    {
        return passwordHash == hashPassword(password);
    }

    static std::string hashPassword(const std::string& plain) 
    {
        std::string hashed = plain;
        char key = 'K';
        for (char& c : hashed) 
        {
            c ^= key;
        }
        return hashed;
    }

    UserId getId() const 
    { 
        return id; 
    }
    std::string getUsername() const 
    { 
        return username; 
    }
    UserType getType() const 
    { 
        return type; 
    }
    const std::vector<OrderId>& getOrderHistory() const 
    { 
        return orderHistory; 
    }

    bool isCustomer() const 
    { 
        return type == UserType::CUSTOMER; 
    }
    bool isSeller() const 
    { 
        return type == UserType::SELLER; 
    }
    bool isAdmin() const 
    { 
        return type == UserType::ADMIN; 
    }

    void addOrder(OrderId orderId) 
    {
        orderHistory.push_back(orderId);
    }

    void writeToStream(std::ostream& os) const 
    {
        os.write(reinterpret_cast<const char*>(&id), sizeof(id));
        
        size_t len = username.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(username.c_str(), len);
        
        len = passwordHash.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(passwordHash.c_str(), len);
        
        int typeInt = static_cast<int>(type);
        os.write(reinterpret_cast<const char*>(&typeInt), sizeof(typeInt));
        
        size_t orderCount = orderHistory.size();
        os.write(reinterpret_cast<const char*>(&orderCount), sizeof(orderCount));
        for (OrderId oid : orderHistory) 
        {
            os.write(reinterpret_cast<const char*>(&oid), sizeof(oid));
        }
    }

    void readFromStream(std::istream& is) 
    {
        is.read(reinterpret_cast<char*>(&id), sizeof(id));
        
        size_t len;
        is.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 0 && len < 100) 
        {
            username.resize(len);
            is.read(&username[0], len);
        }
        
        is.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 0 && len < 100) 
        {
            passwordHash.resize(len);
            is.read(&passwordHash[0], len);
        }
        
        int typeInt;
        is.read(reinterpret_cast<char*>(&typeInt), sizeof(typeInt));
        type = static_cast<UserType>(typeInt);
        
        size_t orderCount;
        is.read(reinterpret_cast<char*>(&orderCount), sizeof(orderCount));
        if (orderCount < 10000) 
        {
            orderHistory.resize(orderCount);
            for (size_t i = 0; i < orderCount; ++i) 
            {
                is.read(reinterpret_cast<char*>(&orderHistory[i]), sizeof(OrderId));
            }
        }
    }

    void display() const 
    {
        std::cout << "User ID: " << id << " | Username: " << username << " | Type: " << (isSeller() ? "Seller" : isAdmin() ? "Admin" : "Customer") << "\n";
    }
};