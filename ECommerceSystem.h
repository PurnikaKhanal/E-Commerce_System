#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cstdio>  
#include "Product.h"
#include "User.h"
#include "Order.h"
#include "Cart.h"
#include "Transaction.h"
#include "ExpenseTracker.h"
#include "CustomerExpenseTracker.h"
#include "DataManager.h"

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

class ECommerceSystem 
{
private:
    std::vector<Product> products;
    std::vector<User> users;
    std::vector<Order> orders;
    std::vector<Transaction> transactions;
    
    Cart cart;
    UserId currentUserId;
    
    std::unique_ptr<ExpenseTracker> sellerTracker;
    std::unique_ptr<CustomerExpenseTracker> customerTracker;

public:
    ECommerceSystem() : currentUserId(0) 
    {
        if (MKDIR("data") != 0 && errno != EEXIST) 
        {
            std::cerr << "Warning: Could not create data directory. Please create it manually.\n";
        }
        
        DataManager::loadSystemState(products, users, orders, transactions);
        initializeTrackers();
    }

    bool login(const std::string& username, const std::string& password) 
    {
        for (const auto& user : users) 
        {
            if (user.getUsername() == username && user.authenticate(password)) 
            {
                currentUserId = user.getId();
                initializeTrackers();
                std::cout << "Welcome back, " << username << "! (ID: " << currentUserId << ")\n";
                return true;
            }
        }
        std::cout << "Invalid username or password. Please try again.\n";
        return false;
    }

    bool registerUser(const std::string& username, const std::string& password, UserType type) 
    {
        if (username.empty() || password.empty()) 
        {
            std::cout << "Username and password cannot be empty.\n";
            return false;
        }

        for (const auto& user : users) 
        {
            if (user.getUsername() == username) 
            {
                std::cout << "Username already exists. Please choose another.\n";
                return false;
            }
        }

        UserId newId = DataManager::getNextUserId(users);
        users.emplace_back(newId, username, password, type);
        
        saveAllData();
        std::cout << "Registration successful! Your user ID is " << newId << ". You can now log in.\n";
        return true;
    }

    void logout() 
    {
        if (isLoggedIn()) 
        {
            std::cout << "Goodbye, " << getCurrentUser().getUsername() << "! Logging out...\n";
            saveCart();
        }
        currentUserId = 0;
        sellerTracker.reset();
        customerTracker.reset();
        cart = Cart();
    }

    void browseProducts() const 
    {
        std::cout << "\n=== AVAILABLE PRODUCTS ===\n";
        if (products.empty()) 
        {
            std::cout << "No products available in the catalog.\n";
            return;
        }

        std::cout << "ID  | Name                           | Price   | Stock | Category         | Seller\n";
        std::cout << "--------------------------------------------------------------------------------\n";
        for (const auto& product : products) 
        {
            printf("%-3d | %-30s | $%-6.2f | %-5d | %-16s | %d\n",
                   product.getId(), 
                   product.getName().substr(0, 30).c_str(),
                   product.getPrice(),
                   product.getStock(),
                   product.getCategory().substr(0, 16).c_str(),
                   product.getSellerId());
        }
        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << "Total products: " << products.size() << "\n";
    }

    void searchProducts(const std::string& query) const 
    {
        if (query.empty()) 
        {
            std::cout << "Search query cannot be empty.\n";
            return;
        }

        std::cout << "\n=== SEARCH RESULTS FOR '" << query << "' ===\n";
        bool found = false;
        std::string queryLower = query;
        std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);
        
        for (const auto& product : products) 
        {
            std::string nameLower = product.getName();
            std::string categoryLower = product.getCategory();
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::tolower);            
            if (nameLower.find(queryLower) != std::string::npos || categoryLower.find(queryLower) != std::string::npos) 
            {
                product.display();
                found = true;
            }
        }
        
        if (!found) 
        {
            std::cout << "No products found matching your search.\n";
        }
    }

    void addToCart(ProductId productId, int quantity) 
    {
        if (!isLoggedIn()) 
        {
            std::cout << "Please log in to add items to cart.\n";
            return;
        }

        if (cart.addItem(productId, quantity, products)) 
        {
            std::cout << "Added to cart successfully!\n";
            saveCart();
        } else 
        {
            std::cout << "Failed to add item to cart.\n";
        }
    }

    void viewCart() const 
    {
        if (!isLoggedIn()) 
        {
            std::cout << "Please log in to view your cart.\n";
            return;
        }
        cart.display(products);
    }

    void removeFromCart(ProductId productId) 
    {
        if (!isLoggedIn()) 
        {
            std::cout << "Please log in to modify your cart.\n";
            return;
        }

        if (cart.removeItem(productId)) 
        {
            std::cout << "Item removed from cart.\n";
            saveCart();
        }
        else 
        {
            std::cout << "Item not found in cart.\n";
        }
    }

    void clearCart() 
    {
        if (!isLoggedIn()) 
        {
            std::cout << "Please log in to modify your cart.\n";
            return;
        }
        
        if (!cart.isEmpty()) 
        {
            char confirm;
            std::cout << "Are you sure you want to clear your cart? (y/N): ";
            std::cin >> confirm;
            std::cin.ignore();
            if (confirm == 'y' || confirm == 'Y') 
            {
                cart.clear();
                saveCart();
            } 
            else 
            {
                std::cout << "Clear cart cancelled.\n";
            }
        } 
        else 
        {
            std::cout << "Your cart is already empty.\n";
        }
    }

    bool placeOrder() 
    {
        if (!isLoggedIn()) 
        {
            std::cout << "Please log in to place an order.\n";
            return false;
        }

        if (cart.isEmpty()) 
        {
            std::cout << "Your cart is empty. Add some items first!\n";
            return false;
        }

        if (!cart.validateStock(products)) 
        {
            std::cout << "Some items in your cart exceed available stock. Please update your cart.\n";
            return false;
        }

        double total = cart.calculateTotal(products);
        std::cout << "Order total: $" << total << "\n";
        char confirm;
        std::cout << "Confirm order placement? (y/N): ";
        std::cin >> confirm;
        std::cin.ignore();
        if (confirm != 'y' && confirm != 'Y') 
        {
            std::cout << "Order cancelled.\n";
            return false;
        }

        TransactionId nextTransId = DataManager::getNextTransactionId(transactions);
        OrderId nextOrderId = DataManager::getNextOrderId(orders);
        
        Order newOrder(currentUserId, cart.getItems(), total);
        newOrder.setId(nextOrderId);
        orders.push_back(newOrder);

        std::vector<UserId> sellerIdsToUpdate;

        for (const auto& item : cart.getItems()) 
        {
            auto productIt = std::find_if(products.begin(), products.end(),
                [item](const Product& p) { return p.getId() == item.productId; });
            
            if (productIt != products.end()) 
            {
                Transaction sale(nextTransId++, currentUserId, item.productId, productIt->getPrice() * item.quantity, TransactionType::SALE, "Purchase: " + productIt->getName());
                transactions.push_back(sale);

                if (customerTracker) 
                {
                    customerTracker->addPurchase(sale);
                }

                // Update seller tracker for the product's owner
                UserId sellerId = productIt->getSellerId();
                bool sellerTrackerFound = false;
                for (auto& user : users) 
                {
                    if (user.getId() == sellerId && user.isSeller()) 
                    {
                        sellerIdsToUpdate.push_back(sellerId);
                        sellerTrackerFound = true;
                        break;
                    }
                }
                
                if (sellerTrackerFound) 
                {
                    // We need to update the seller's tracker
                    ExpenseTracker tempTracker(sellerId);
                    tempTracker.loadTransactions(transactions);
                    tempTracker.addSale(sale);
                    // Save the seller's transactions (simplified - in production use a map of trackers)
                }

                // Update product stock
                productIt->reduceStock(item.quantity);
            }
        }

        for (auto& user : users) 
        {
            if (user.getId() == currentUserId) 
            {
                user.addOrder(newOrder.getId());
                std::cout << "Order #" << newOrder.getId() << " placed successfully!\n";
                break;
            }
        }

        cart.clear();
        saveAllData();
        
        return true;
    }

    void viewOrderHistory() const 
    {
        if (!isLoggedIn()) 
        {
            std::cout << "Please log in to view order history.\n";
            return;
        }

        std::cout << "\n=== YOUR ORDER HISTORY ===\n";
        bool found = false;
        
        for (const auto& order : orders) 
        {
            if (order.getUserId() == currentUserId) 
            {
                order.display(products);
                std::cout << "------------------------\n";
                found = true;
            }
        }
        
        if (!found) 
        {
            std::cout << "No orders found.\n";
        }
    }

    void addProduct(const std::string& name, double price, const std::string& category, int stock) 
    {
        if (!isLoggedIn() || !getCurrentUser().isSeller()) 
        {
            std::cout << "Only sellers can add products.\n";
            return;
        }

        if (name.empty() || category.empty() || stock <= 0 || price < 0) 
        {
            std::cout << "Invalid product parameters. Please check your input.\n";
            return;
        }

        ProductId newId = DataManager::getNextProductId(products);
        products.emplace_back(newId, name, price, category, stock, currentUserId);
        
        saveAllData();
        std::cout << "Product added successfully! Product ID: " << newId << "\n";
    }

    void recordExpense(double amount, const std::string& description) 
    {
        if (!isLoggedIn() || !getCurrentUser().isSeller()) 
        {
            std::cout << "Only sellers can record expenses.\n";
            return;
        }

        if (amount <= 0) 
        {
            std::cout << "Expense amount must be positive.\n";
            return;
        }

        if (sellerTracker) 
        {
            TransactionId newId = DataManager::getNextTransactionId(transactions);
            sellerTracker->addExpense(amount, description, newId);
            
            Transaction expense(newId, currentUserId, -1, -amount,
                              TransactionType::EXPENSE, description);
            transactions.push_back(expense);
            
            saveAllData();
            std::cout << "Expense recorded successfully!\n";
        }
    }

    void processRefund(OrderId orderId) 
    {
        if (!isLoggedIn() || !getCurrentUser().isAdmin()) 
        {
            std::cout << "Only administrators can process refunds.\n";
            return;
        }

        auto orderIt = std::find_if(orders.begin(), orders.end(), [orderId](const Order& o) { return o.getId() == orderId; });
        
        if (orderIt == orders.end()) 
        {
            std::cout << "Order not found.\n";
            return;
        }

        if (orderIt->getStatus() == "Refunded") 
        {
            std::cout << "This order has already been refunded.\n";
            return;
        }

        char confirm;
        std::cout << "Refund order #" << orderId << " for $" << orderIt->getTotal() << "? (y/N): ";
        std::cin >> confirm;
        std::cin.ignore();
        if (confirm != 'y' && confirm != 'Y') 
        {
            std::cout << "Refund cancelled.\n";
            return;
        }

        TransactionId refundId = DataManager::getNextTransactionId(transactions);
        Transaction refund(refundId, orderIt->getUserId(), -1, -orderIt->getTotal(), TransactionType::REFUND, "Refund for Order #" + std::to_string(orderId));
        transactions.push_back(refund);

        // Update order status
        const_cast<Order&>(*orderIt).setStatus("Refunded");

        // Update customer tracker
        for (auto& user : users) 
        {
            if (user.getId() == orderIt->getUserId() && user.isCustomer()) 
            {
                CustomerExpenseTracker tracker(orderIt->getUserId());
                tracker.loadSpendingHistory(transactions);
                tracker.addRefund(refund);
                break;
            }
        }

        saveAllData();
        std::cout << "Refund processed successfully for Order #" << orderId << ".\n";
    }

    void viewSellerReport() const 
    {
        if (!isLoggedIn() || !getCurrentUser().isSeller()) 
        {
            std::cout << "Only sellers can view reports.\n";
            return;
        }

        if (sellerTracker) 
        {
            sellerTracker->displaySummary();
        }
    }

    void viewSellerDetailedReport() const 
    {
        if (!isLoggedIn() || !getCurrentUser().isSeller()) 
        {
            std::cout << "Only sellers can view detailed reports.\n";
            return;
        }

        if (sellerTracker) 
        {
            sellerTracker->displayDetailedReport();
        }
    }

    void viewSpendingSummary() const 
    {
        if (!isLoggedIn() || !getCurrentUser().isCustomer()) 
        {
            std::cout << "Only customers can view spending summary.\n";
            return;
        }

        if (customerTracker) 
        {
            customerTracker->displaySpendingSummary();
        }
    }

    void viewSpendingHistory() const 
    {
        if (!isLoggedIn() || !getCurrentUser().isCustomer()) 
        {
            std::cout << "Only customers can view spending history.\n";
            return;
        }

        if (customerTracker) 
        {
            customerTracker->displayDetailedHistory();
            customerTracker->displayMonthlySummary();
        }
    }

    bool isLoggedIn() const 
    { 
        return currentUserId != 0; 
    }
    
    const User& getCurrentUser() const 
    {
        for (const auto& user : users) 
        {
            if (user.getId() == currentUserId) 
            {
                return user;
            }
        }
        throw std::runtime_error("Current user not found in system");
    }

    void initializeTrackers() 
    {
        if (!isLoggedIn()) 
        {
            sellerTracker.reset();
            customerTracker.reset();
            return;
        }

        const User& user = getCurrentUser();
        loadCart();

        if (user.isSeller()) 
        {
            sellerTracker = std::make_unique<ExpenseTracker>(currentUserId);
            sellerTracker->loadTransactions(transactions);
        } 
        else if (user.isCustomer()) 
        {
            customerTracker = std::make_unique<CustomerExpenseTracker>(currentUserId);
            customerTracker->loadSpendingHistory(transactions);
        }
    }

    void saveCart() 
    {
        if (isLoggedIn()) 
        {
            cart.saveToFile();
        }
    }

    void loadCart() 
    {
        if (isLoggedIn()) 
        {
            cart = Cart(currentUserId);
            cart.loadFromFile();
        }
    }

    void saveAllData() 
    {
        DataManager::saveSystemState(products, users, orders, transactions);
        saveCart();
    }

    ~ECommerceSystem() 
    {
        saveAllData();
    }
}; 