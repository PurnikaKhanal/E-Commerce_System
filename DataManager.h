#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <iostream>
#include "Product.h"
#include "User.h"
#include "Order.h"
#include "Transaction.h"
#include "config.h"

class DataManager {
public:
    static void loadSystemState(std::vector<Product>& products, std::vector<User>& users, 
                               std::vector<Order>& orders, std::vector<Transaction>& transactions) {
        std::cout << "Loading system state...\n";
        loadProducts(products);
        loadUsers(users);
        loadOrders(orders);
        loadTransactions(transactions);
        std::cout << "System state loaded successfully.\n";
    }

    static void saveSystemState(const std::vector<Product>& products, const std::vector<User>& users,
                               const std::vector<Order>& orders, const std::vector<Transaction>& transactions) {
        std::cout << "Saving system state...\n";
        saveProducts(products);
        saveUsers(users);
        saveOrders(orders);
        saveTransactions(transactions);
        std::cout << "System state saved successfully.\n";
    }

    static void loadProducts(std::vector<Product>& products) {
        products.clear();
        std::ifstream ifs(PRODUCT_FILE, std::ios::binary);
        if (!ifs.is_open()) {
            std::cout << "Info: No existing product file found. Starting with empty inventory.\n";
            return;
        }

        try {
            while (ifs.peek() != EOF) {
                Product p;
                p.readFromStream(ifs);
                if (ifs.good()) {
                    products.push_back(p);
                }
            }
            std::cout << "Loaded " << products.size() << " products.\n";
        } catch (const std::exception& e) {
            std::cerr << "Error loading products: " << e.what() << "\n";
            std::cerr << "Some products may not have been loaded. File might be corrupted.\n";
        }
    }

    static void saveProducts(const std::vector<Product>& products) {
        atomicWrite(PRODUCT_FILE, [&](std::ofstream& ofs) {
            for (const auto& p : products) {
                p.writeToStream(ofs);
            }
        });
        std::cout << "Saved " << products.size() << " products.\n";
    }

    static void loadUsers(std::vector<User>& users) {
        users.clear();
        std::ifstream ifs(USER_FILE, std::ios::binary);
        if (!ifs.is_open()) {
            std::cout << "Info: No existing user file found. Creating default admin user.\n";
            users.push_back(User(1, "admin", "admin123", UserType::ADMIN));
            std::cout << "Default admin created (Username: admin, Password: admin123).\n";
            return;
        }

        try {
            while (ifs.peek() != EOF) {
                User u;
                u.readFromStream(ifs);
                if (ifs.good()) {
                    users.push_back(u);
                }
            }
            std::cout << "Loaded " << users.size() << " users.\n";
        } catch (const std::exception& e) {
            std::cerr << "Error loading users: " << e.what() << "\n";
        }
    }

    static void saveUsers(const std::vector<User>& users) {
        atomicWrite(USER_FILE, [&](std::ofstream& ofs) {
            for (const auto& u : users) {
                u.writeToStream(ofs);
            }
        });
        std::cout << "Saved " << users.size() << " users.\n";
    }

    static void loadOrders(std::vector<Order>& orders) 
    {
        orders.clear();
        std::ifstream ifs(ORDER_FILE, std::ios::binary);
        if (!ifs.is_open()) 
        {
            std::cout << "Info: No existing order file found.\n";
            return;
        }

        try 
        {
            OrderId maxId = 0;
            while (ifs.peek() != EOF) 
            {
                Order o = Order::readFromStream(ifs);
                if (ifs.good()) 
                {
                    orders.push_back(o);
                    if (o.getId() > maxId) maxId = o.getId();
                }
            }
            std::cout << "Loaded " << orders.size() << " orders.\n";
            // Set the next order ID to avoid collisions
            if (!orders.empty()) 
            {
                Order::setNextId(maxId + 1);
            }
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Error loading orders: " << e.what() << "\n";
        }
    }

    static void saveOrders(const std::vector<Order>& orders) 
    {
        atomicWrite(ORDER_FILE, [&](std::ofstream& ofs) 
        {
            for (const auto& o : orders) 
            {
                o.writeToStream(ofs);
            }
        });
        std::cout << "Saved " << orders.size() << " orders.\n";
    }

    static void loadTransactions(std::vector<Transaction>& transactions) 
    {
        transactions.clear();
        std::ifstream ifs(TRANSACTION_FILE, std::ios::binary);
        if (!ifs.is_open()) 
        {
            std::cout << "Info: No existing transaction file found.\n";
            return;
        }

        try 
        {
            while (ifs.peek() != EOF) 
            {
                Transaction t = Transaction::readFromStream(ifs);
                if (ifs.good()) 
                {
                    transactions.push_back(t);
                }
            }
            std::cout << "Loaded " << transactions.size() << " transactions.\n";
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Error loading transactions: " << e.what() << "\n";
        }
    }

    static void saveTransactions(const std::vector<Transaction>& transactions) 
    {
        atomicWrite(TRANSACTION_FILE, [&](std::ofstream& ofs) 
        {
            for (const auto& t : transactions) 
            {
                t.writeToStream(ofs);
            }
        });
        std::cout << "Saved " << transactions.size() << " transactions.\n";
    }

    static TransactionId getNextTransactionId(const std::vector<Transaction>& transactions) 
    {
        TransactionId maxId = 0;
        for (const auto& t : transactions) 
        {
            if (t.getId() > maxId) maxId = t.getId();
        }
        return maxId + 1;
    }

    static OrderId getNextOrderId(const std::vector<Order>& orders) 
    {
        OrderId maxId = 0;
        for (const auto& o : orders) 
        {
            if (o.getId() > maxId) maxId = o.getId();
        }
        return maxId + 1;
    }

    static UserId getNextUserId(const std::vector<User>& users) 
    {
        UserId maxId = 0;
        for (const auto& u : users) 
        {
            if (u.getId() > maxId) maxId = u.getId();
        }
        return maxId + 1;
    }

    static ProductId getNextProductId(const std::vector<Product>& products) 
    {
        ProductId maxId = 0;
        for (const auto& p : products) 
        {
            if (p.getId() > maxId) maxId = p.getId();
        }
        return maxId + 1;
    }

private:
    static void atomicWrite(const std::string& filename, const std::function<void(std::ofstream&)>& writer) 
    {
        std::string tempFile = filename + ".tmp";
        
        try 
        {
            std::ofstream ofs(tempFile, std::ios::binary);
            if (!ofs.is_open()) 
            {
                throw std::runtime_error("Cannot open temporary file: " + tempFile);
            }
            
            writer(ofs);
            ofs.close();
            
            if (ofs.fail()) 
            {
                throw std::runtime_error("Error writing to temporary file: " + tempFile);
            }
            
            if (std::remove(filename.c_str()) != 0 && errno != ENOENT) 
            {
                throw std::runtime_error("Cannot remove original file: " + filename);
            }
            
            if (std::rename(tempFile.c_str(), filename.c_str()) != 0) 
            {
                throw std::runtime_error("Cannot rename temporary file to: " + filename);
            }
            
        } 
        catch (const std::exception& e) 
        {
            std::remove(tempFile.c_str());
            std::cerr << "File operation error: " << e.what() << "\n";
            throw;
        }
    }
};