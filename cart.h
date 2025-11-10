#pragma once
#include <vector>
#include <fstream>  
#include <iostream>
#include <algorithm>
#include "Product.h"
#include "config.h"

class Cart 
{
private:
    UserId userId;
    std::vector<CartItem> items;

public:
    explicit Cart(UserId userId = -1) : userId(userId) {}

    bool addItem(ProductId productId, int quantity, const std::vector<Product>& products) 
    {
        auto productIt = std::find_if(products.begin(), products.end(), [productId](const Product& p) { return p.getId() == productId; });
        
        if (productIt == products.end()) 
        {
            std::cout << "Product not found!\n";
            return false;
        }

        if (quantity <= 0 || quantity > productIt->getStock()) 
        {
            std::cout << "Invalid quantity! Available stock: " << productIt->getStock() << "\n";
            return false;
        }

        auto itemIt = std::find_if(items.begin(), items.end(), [productId](const CartItem& item) { return item.productId == productId; });

        if (itemIt != items.end()) 
        {
            if (itemIt->quantity + quantity > productIt->getStock()) 
            {
                std::cout << "Cannot add " << quantity << " more. Available stock: " << productIt->getStock() - itemIt->quantity << "\n";
                return false;
            }
            itemIt->quantity += quantity;
        } 
        else 
        {
            items.push_back({productId, quantity});
        }

        return true;
    }

    bool removeItem(ProductId productId) 
    {
        auto it = std::remove_if(items.begin(), items.end(), [productId](const CartItem& item) { return item.productId == productId; });
        
        if (it != items.end()) 
        {
            items.erase(it, items.end());
            return true;
        }
        std::cout << "Item not found in cart.\n";
        return false;
    }

    void clear() 
    { 
        items.clear(); 
        std::cout << "Cart cleared successfully.\n";
    }
    
    int getItemCount() const 
    { 
        return items.size(); 
    }
    const std::vector<CartItem>& getItems() const 
    { 
        return items; 
    }
    bool isEmpty() const 
    { 
        return items.empty(); 
    }

    double calculateTotal(const std::vector<Product>& products) const 
    {
        double total = 0.0;
        for (const auto& item : items) 
        {
            auto productIt = std::find_if(products.begin(), products.end(), [item](const Product& p) { return p.getId() == item.productId; });
            
            if (productIt != products.end()) 
            {
                total += productIt->getPrice() * item.quantity;
            }
        }
        return total;
    }

    bool validateStock(const std::vector<Product>& products) const 
    {
        for (const auto& item : items) 
        {
            // Use explicit iterator type instead of auto
            std::vector<Product>::const_iterator productIt = std::find_if(products.begin(), products.end(), [item](const Product& p) { return p.getId() == item.productId; });
            if (productIt == products.end() || item.quantity > productIt->getStock()) 
            {
                std::cout << "Stock validation failed for product ID: " << item.productId << "\n";
                return false;
            }
        }
        return true;
    }

    void display(const std::vector<Product>& products) const 
    {
        if (items.empty()) 
        {
            std::cout << "Your cart is empty.\n";
            return;
        }

        std::cout << "\n=== YOUR SHOPPING CART ===\n";
        std::cout << "Product ID | Name                 | Price    | Quantity | Subtotal\n";
        std::cout << "---------------------------------------------------------------\n";
        
        for (const auto& item : items) 
        {
            std::vector<Product>::const_iterator productIt = std::find_if(products.begin(), products.end(), [item](const Product& p) { return p.getId() == item.productId; });
            if (productIt != products.end()) 
            {
                double subtotal = productIt->getPrice() * item.quantity;
                std::string name = productIt->getName();
                if (name.length() > 20) name = name.substr(0, 17) + "...";
                printf("%-10d | %-20s | $%-7.2f | %-8d | $%-7.2f\n", item.productId, name.c_str(), productIt->getPrice(), item.quantity, subtotal);
            }
        }
        
        std::cout << "---------------------------------------------------------------\n";
        printf("Total: $%.2f\n", calculateTotal(products));
    }

    void saveToFile() const {
        if (userId == -1) return;
        
        std::string filename = std::string(CART_FILE_PREFIX) + std::to_string(userId) + ".dat";
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) {
            std::cerr << "Warning: Could not save cart to " << filename << "\n";
            return;
        }

        size_t count = items.size();
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& item : items) {
            ofs.write(reinterpret_cast<const char*>(&item.productId), sizeof(item.productId));
            ofs.write(reinterpret_cast<const char*>(&item.quantity), sizeof(item.quantity));
        }
        
        std::cout << "Cart saved successfully.\n";
    }

    void loadFromFile() {
        if (userId == -1) return;
        
        std::string filename = std::string(CART_FILE_PREFIX) + std::to_string(userId) + ".dat";
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) {
            return; // No cart file exists yet, which is normal
        }

        size_t count;
        ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
        if (ifs.fail()) {
            std::cerr << "Warning: Cart file corrupted: " << filename << "\n";
            return;
        }
        
        items.resize(count);
        for (size_t i = 0; i < count; ++i) {
            ifs.read(reinterpret_cast<char*>(&items[i].productId), sizeof(items[i].productId));
            ifs.read(reinterpret_cast<char*>(&items[i].quantity), sizeof(items[i].quantity));
            if (ifs.fail()) {
                std::cerr << "Warning: Cart file read error for item " << i << "\n";
                items.resize(i); // Keep partial data
                break;
            }
        }
        
        std::cout << "Cart loaded (" << items.size() << " items).\n";
    }
};