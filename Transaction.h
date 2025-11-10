#pragma once
#include <string>
#include <ctime>
#include <iostream>
#include <cstring>
#include "config.h"

class Transaction {
private:
    TransactionId id;
    UserId userId;
    ProductId productId;
    double amount;
    TransactionType type;
    std::string description;
    char timestamp[DATE_STR_LEN];

public:
    Transaction() : id(0), userId(0), productId(-1), amount(0.0), type(TransactionType::SALE) 
    {
        std::strncpy(timestamp, "1970-01-01 00:00:00", DATE_STR_LEN);
    }

    Transaction(TransactionId id, UserId userId, ProductId productId, double amount, TransactionType type, const std::string& description) : id(id), userId(userId), productId(productId), amount(amount), type(type), description(description) 
    {
        updateTimestamp();
    }

    void updateTimestamp() 
    {
        std::time_t now = std::time(nullptr);
        std::strftime(timestamp, DATE_STR_LEN, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    }

    TransactionId getId() const 
    { 
        return id; 
    }
    UserId getUserId() const 
    { 
        return userId; 
    }
    ProductId getProductId() const 
    { 
        return productId; 
    }
    double getAmount() const 
    { 
        return amount; 
    }
    TransactionType getType() const 
    { 
        return type; 
    }
    std::string getDescription() const 
    { 
        return description; 
    }
    const char* getTimestamp() const 
    { 
        return timestamp; 
    }

    bool isSale() const 
    { 
        return type == TransactionType::SALE; 
    }
    bool isRefund() const 
    { 
        return type == TransactionType::REFUND; 
    }
    bool isExpense() const 
    { 
        return type == TransactionType::EXPENSE; 
    }

    void writeToStream(std::ostream& os) const 
    {
        os.write(reinterpret_cast<const char*>(&id), sizeof(id));
        os.write(reinterpret_cast<const char*>(&userId), sizeof(userId));
        os.write(reinterpret_cast<const char*>(&productId), sizeof(productId));
        os.write(reinterpret_cast<const char*>(&amount), sizeof(amount));
        
        int typeInt = static_cast<int>(type);
        os.write(reinterpret_cast<const char*>(&typeInt), sizeof(typeInt));
        
        size_t descLen = description.size();
        os.write(reinterpret_cast<const char*>(&descLen), sizeof(descLen));
        os.write(description.c_str(), descLen);
        
        os.write(timestamp, DATE_STR_LEN);
    }

    static Transaction readFromStream(std::istream& is) 
    {
        TransactionId id; UserId userId; ProductId productId; double amount; int typeInt;
        size_t descLen; char timestamp[DATE_STR_LEN];

        is.read(reinterpret_cast<char*>(&id), sizeof(id));
        is.read(reinterpret_cast<char*>(&userId), sizeof(userId));
        is.read(reinterpret_cast<char*>(&productId), sizeof(productId));
        is.read(reinterpret_cast<char*>(&amount), sizeof(amount));
        is.read(reinterpret_cast<char*>(&typeInt), sizeof(typeInt));
        is.read(reinterpret_cast<char*>(&descLen), sizeof(descLen));
        
        if (descLen > 0 && descLen < 10000) 
        {
            std::string description(descLen, '\0');
            is.read(&description[0], descLen);
            is.read(timestamp, DATE_STR_LEN);
            return Transaction(id, userId, productId, amount, static_cast<TransactionType>(typeInt), description);
        }
        
        return Transaction(); // Return default on error
    }

    void display() const 
    {
        std::string typeStr;
        switch (type) 
        {
            case TransactionType::SALE: typeStr = "SALE"; break;
            case TransactionType::REFUND: typeStr = "REFUND"; break;
            case TransactionType::EXPENSE: typeStr = "EXPENSE"; break;
            case TransactionType::DEPOSIT: typeStr = "DEPOSIT"; break;
        }
        
        std::cout << "[" << timestamp << "] " << typeStr << " | User: " << userId << " | Amount: $" << amount << " | " << description << "\n";
    }
};