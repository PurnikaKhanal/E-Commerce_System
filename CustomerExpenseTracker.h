#pragma once
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "Transaction.h"
#include "config.h"

class CustomerExpenseTracker {
private:
    UserId customerId;
    std::vector<Transaction> spendingHistory;

public:
    explicit CustomerExpenseTracker(UserId customerId) : customerId(customerId) {}

    void loadSpendingHistory(const std::vector<Transaction>& allTransactions) {
        spendingHistory.clear();
        for (const auto& t : allTransactions) {
            if (t.getUserId() == customerId && (t.isSale() || t.isRefund())) {
                spendingHistory.push_back(t);
            }
        }
    }

    void addPurchase(const Transaction& purchase) {
        if (purchase.isSale() && purchase.getUserId() == customerId) {
            spendingHistory.push_back(purchase);
        }
    }

    void addRefund(const Transaction& refund) {
        if (refund.isRefund() && refund.getUserId() == customerId) {
            spendingHistory.push_back(refund);
        }
    }

    double getTotalSpent() const {
        double total = 0.0;
        for (const auto& t : spendingHistory) {
            if (t.isSale()) total += t.getAmount();
        }
        return total;
    }

    double getTotalRefunded() const {
        double total = 0.0;
        for (const auto& t : spendingHistory) {
            if (t.isRefund()) total += t.getAmount();
        }
        return total;
    }

    double getNetSpent() const {
        return getTotalSpent() - getTotalRefunded();
    }

    std::vector<Transaction> getSpendingByDate(const std::string& startDate, 
                                              const std::string& endDate) const {
        std::vector<Transaction> result;
        for (const auto& t : spendingHistory) {
            std::string date = std::string(t.getTimestamp());
            if (date.length() >= 10) {
                date = date.substr(0, 10);
                if (date >= startDate && date <= endDate) {
                    result.push_back(t);
                }
            }
        }
        return result;
    }

    std::map<std::string, double> getMonthlySummary() const {
        std::map<std::string, double> summary;
        for (const auto& t : spendingHistory) {
            if (t.isSale()) {
                std::string ts = std::string(t.getTimestamp());
                if (ts.length() >= 7) {
                    std::string month = ts.substr(0, 7);
                    summary[month] += t.getAmount();
                }
            }
        }
        return summary;
    }

    void displaySpendingSummary() const {
        std::cout << "\n=== CUSTOMER SPENDING SUMMARY ===\n";
        std::cout << "Total Spent: $" << getTotalSpent() << "\n";
        std::cout << "Total Refunded: $" << getTotalRefunded() << "\n";
        std::cout << "Net Spent: $" << getNetSpent() << "\n";
    }

    void displayDetailedHistory() const {
        std::cout << "\n=== SPENDING HISTORY ===\n";
        std::cout << "Date       | Type    | Amount  | Description\n";
        std::cout << "--------------------------------------------\n";
        
        for (const auto& t : spendingHistory) {
            std::string date = std::string(t.getTimestamp());
            if (date.length() >= 10) date = date.substr(0, 10);
            std::string type = t.isSale() ? "PURCHASE" : "REFUND";
            
            printf("%-10s | %-7s | $%-6.2f | %s\n", 
                   date.c_str(), type.c_str(), t.getAmount(), t.getDescription().c_str());
        }
    }

    void displayMonthlySummary() const {
        auto monthly = getMonthlySummary();
        std::cout << "\n=== MONTHLY SPENDING SUMMARY ===\n";
        std::cout << "Month     | Amount Spent\n";
        std::cout << "---------------------------\n";
        
        for (const auto& [month, amount] : monthly) {
            printf("%-9s | $%-8.2f\n", month.c_str(), amount);
        }
    }

    const std::vector<Transaction>& getSpendingHistory() const {
        return spendingHistory;
    }
};