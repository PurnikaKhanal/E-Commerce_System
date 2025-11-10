#pragma once
#include <string>
#include <iostream>
#include <algorithm>
#include "config.h"

class Product 
{
private:
    ProductId id;
    std::string name;
    double price;
    std::string category;
    int stock;
    UserId sellerId;

public:
    Product() : id(0), name("Unknown"), price(0.0), category("Misc"), stock(0), sellerId(0) {}
    
    Product(ProductId id, const std::string& name, double price, const std::string& category, int stock, UserId sellerId = 0) : id(id), name(name), price(price), category(category), stock(stock), sellerId(sellerId) {}

    ProductId getId() const 
    { 
        return id; 
    }
    std::string getName() const 
    { 
        return name; 
    }
    double getPrice() const 
    { 
        return price; 
    }
    std::string getCategory() const 
    { 
        return category; 
    }
    int getStock() const 
    { 
        return stock; 
    }
    UserId getSellerId() const 
    { 
        return sellerId; 
    }

    void setStock(int newStock) 
    { 
        if (newStock >= 0) stock = newStock; 
    }
    
    void setPrice(double newPrice) 
    { 
        if (newPrice >= 0) price = newPrice; 
    }

    void setName(const std::string& newName) 
    { 
        if (!newName.empty()) name = newName; 
    }

    bool reduceStock(int quantity) 
    {
        if (quantity > 0 && stock >= quantity) 
        {
            stock -= quantity;
            return true;
        }
        std::cout << "Insufficient stock for product '" << name << "'. Available: " << stock << "\n";
        return false;
    }

    void restock(int quantity) 
    {
        if (quantity > 0) 
        {
            stock += quantity;
            std::cout << "Restocked " << quantity << " units of '" << name << "'. New stock: " << stock << "\n";
        }
    }

    bool isInStock() const 
    { 
        return stock > 0; 
    }

    void display() const 
    {
        std::cout << "ID: " << id << " | " << name << " | $" << price << " | Stock: " << stock << " | Category: " << category << " | Seller: " << sellerId << "\n";
    }

    void writeToStream(std::ostream& os) const 
    {
        os.write(reinterpret_cast<const char*>(&id), sizeof(id));
        
        size_t len = name.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(name.c_str(), len);
        
        os.write(reinterpret_cast<const char*>(&price), sizeof(price));
        
        len = category.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(category.c_str(), len);
        
        os.write(reinterpret_cast<const char*>(&stock), sizeof(stock));
        os.write(reinterpret_cast<const char*>(&sellerId), sizeof(sellerId));
    }

    void readFromStream(std::istream& is) 
    {
        is.read(reinterpret_cast<char*>(&id), sizeof(id));
        
        size_t len;
        is.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 0 && len < 1000) 
        {
            name.resize(len);
            is.read(&name[0], len);
        }
        
        is.read(reinterpret_cast<char*>(&price), sizeof(price));
        
        is.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 0 && len < 1000) 
        {
            category.resize(len);
            is.read(&category[0], len);
        }
        
        is.read(reinterpret_cast<char*>(&stock), sizeof(stock));
        is.read(reinterpret_cast<char*>(&sellerId), sizeof(sellerId));
    }
};