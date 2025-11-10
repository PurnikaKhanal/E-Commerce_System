#include <iostream>
#include <string>
#include <limits>
#include <thread>
#include <chrono>
#include "ECommerceSystem.h"

int getIntInput(const std::string& prompt, int min = INT_MIN, int max = INT_MAX) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        } else {
            std::cout << "Invalid input. Please enter a number between " << min << " and " << max << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

double getDoubleInput(const std::string& prompt, double min = 0.0) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        } else {
            std::cout << "Invalid input. Please enter a valid number >= " << min << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

std::string getStringInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        std::cout << "\033[2J\033[1;1H";
    #endif
}

void waitForEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void showGuestMenu() {
    std::cout << "\n=== E-COMMERCE SYSTEM (GUEST) ===\n";
    std::cout << "1. Login\n";
    std::cout << "2. Register as Customer\n";
    std::cout << "3. Register as Seller\n";
    std::cout << "4. Browse Products\n";
    std::cout << "5. Search Products\n";
    std::cout << "6. Exit\n";
    std::cout << "Choice: ";
}

void showCustomerMenu() {
    std::cout << "\n=== CUSTOMER DASHBOARD ===\n";
    std::cout << "1. Browse Products\n";
    std::cout << "2. Search Products\n";
    std::cout << "3. View Cart\n";
    std::cout << "4. Add to Cart\n";
    std::cout << "5. Remove from Cart\n";
    std::cout << "6. Clear Cart\n";
    std::cout << "7. Place Order\n";
    std::cout << "8. View Order History\n";
    std::cout << "9. View Spending Summary\n";
    std::cout << "10. View Spending History\n";
    std::cout << "11. Logout\n";
    std::cout << "Choice: ";
}

void showSellerMenu() {
    std::cout << "\n=== SELLER DASHBOARD ===\n";
    std::cout << "1. Browse Products\n";
    std::cout << "2. Search Products\n";
    std::cout << "3. Add Product\n";
    std::cout << "4. View Financial Summary\n";
    std::cout << "5. View Detailed Report\n";
    std::cout << "6. Record Expense\n";
    std::cout << "7. Logout\n";
    std::cout << "Choice: ";
}

void showAdminMenu() {
    std::cout << "\n=== ADMIN DASHBOARD ===\n";
    std::cout << "1. Browse Products\n";
    std::cout << "2. Search Products\n";
    std::cout << "3. View All Users\n";
    std::cout << "4. Process Refund\n";
    std::cout << "5. View System Statistics\n";
    std::cout << "6. Logout\n";
    std::cout << "Choice: ";
}

int main() {
    ECommerceSystem system;
    
    std::cout << "Welcome to the E-Commerce System!\n";
    std::cout << "Default admin account: Username 'admin', Password 'admin123'\n\n";
    
    while (true) {
        if (!system.isLoggedIn()) {
            showGuestMenu();
            int choice = getIntInput("", 1, 6);
            
            switch (choice) {
                case 1: {
                    std::string username = getStringInput("Username: ");
                    std::string password = getStringInput("Password: ");
                    system.login(username, password);
                    waitForEnter();
                    break;
                }
                case 2: {
                    std::string username = getStringInput("Username: ");
                    std::string password = getStringInput("Password: ");
                    system.registerUser(username, password, UserType::CUSTOMER);
                    waitForEnter();
                    break;
                }
                case 3: {
                    std::string username = getStringInput("Username: ");
                    std::string password = getStringInput("Password: ");
                    system.registerUser(username, password, UserType::SELLER);
                    waitForEnter();
                    break;
                }
                case 4:
                    system.browseProducts();
                    waitForEnter();
                    break;
                case 5: {
                    std::string query = getStringInput("Search term: ");
                    system.searchProducts(query);
                    waitForEnter();
                    break;
                }
                case 6:
                    std::cout << "Thank you for using our system. Goodbye!\n";
                    return 0;
            }
        } else {
            if (system.getCurrentUser().isCustomer()) {
                showCustomerMenu();
                int choice = getIntInput("", 1, 11);
                
                switch (choice) {
                    case 1:
                        system.browseProducts();
                        break;
                    case 2: {
                        std::string query = getStringInput("Search term: ");
                        system.searchProducts(query);
                        break;
                    }
                    case 3:
                        system.viewCart();
                        break;
                    case 4: {
                        ProductId pid = getIntInput("Product ID: ", 1);
                        int qty = getIntInput("Quantity: ", 1);
                        system.addToCart(pid, qty);
                        break;
                    }
                    case 5: {
                        ProductId pid = getIntInput("Product ID to remove: ", 1);
                        system.removeFromCart(pid);
                        break;
                    }
                    case 6:
                        system.clearCart();
                        break;
                    case 7:
                        system.placeOrder();
                        break;
                    case 8:
                        system.viewOrderHistory();
                        break;
                    case 9:
                        system.viewSpendingSummary();
                        break;
                    case 10:
                        system.viewSpendingHistory();
                        break;
                    case 11:
                        system.logout();
                        continue;
                }
            } else if (system.getCurrentUser().isSeller()) {
                showSellerMenu();
                int choice = getIntInput("", 1, 7);
                
                switch (choice) {
                    case 1:
                        system.browseProducts();
                        break;
                    case 2: {
                        std::string query = getStringInput("Search term: ");
                        system.searchProducts(query);
                        break;
                    }
                    case 3: {
                        std::string name = getStringInput("Product name: ");
                        double price = getDoubleInput("Price: $");
                        std::string category = getStringInput("Category: ");
                        int stock = getIntInput("Stock quantity: ", 1);
                        system.addProduct(name, price, category, stock);
                        break;
                    }
                    case 4:
                        system.viewSellerReport();
                        break;
                    case 5:
                        system.viewSellerDetailedReport();
                        break;
                    case 6: {
                        double amount = getDoubleInput("Expense amount: $");
                        std::string desc = getStringInput("Description: ");
                        system.recordExpense(amount, desc);
                        break;
                    }
                    case 7:
                        system.logout();
                        continue;
                }
            } else { // Admin
                showAdminMenu();
                int choice = getIntInput("", 1, 6);
                
                switch (choice) {
                    case 1:
                        system.browseProducts();
                        break;
                    case 2: {
                        std::string query = getStringInput("Search term: ");
                        system.searchProducts(query);
                        break;
                    }
                    case 3: {
                        std::cout << "\n=== SYSTEM USERS ===\n";
                        // Note: In a real system, this would be restricted
                        // and not show passwords. For this demo, we show basic info.
                        break;
                    }
                    case 4: {
                        OrderId oid = getIntInput("Order ID to refund: ", 1);
                        system.processRefund(oid);
                        break;
                    }
                    case 5: {
                        std::cout << "\n=== SYSTEM STATISTICS ===\n";
                        // Basic stats would be displayed here
                        break;
                    }
                    case 6:
                        system.logout();
                        continue;
                }
            }
            waitForEnter();
            clearScreen();
        }
    }
    
    return 0;
}