#pragma once
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include "Transaction.h"
#include "config.h"

class ExpenseTracker {
private:
    UserId sellerId;
    std::vector<Transaction> transactions;

public:
    explicit ExpenseTracker(UserId sellerId) : sellerId(sellerId) {}

    void loadTransactions(const std::vector<Transaction>& allTransactions) {
        transactions.clear();
        for (const auto& t : allTransactions) {
            if (t.getUserId() == sellerId && 
                (t.isSale() || t.isExpense() || t.isRefund())) {
                transactions.push_back(t);
            }
        }
    }

    void addSale(const Transaction& sale) {
        if (sale.isSale() && sale.getUserId() == sellerId) {
            transactions.push_back(sale);
        }
    }

    void addExpense(double amount, const std::string& description, TransactionId newId) {
        if (amount > 0) {
            Transaction expense(newId, sellerId, -1, -amount, 
                              TransactionType::EXPENSE, description);
            transactions.push_back(expense);
        }
    }

    void addRefund(const Transaction& refund) {
        if (refund.isRefund() && refund.getUserId() == sellerId) {
            transactions.push_back(refund);
        }
    }

    double getTotalRevenue() const {
        double total = 0.0;
        for (const auto& t : transactions) {
            if (t.isSale()) total += t.getAmount();
        }
        return total;
    }

    double getTotalExpenses() const {
        double total = 0.0;
        for (const auto& t : transactions) {
            if (t.isExpense()) total += std::abs(t.getAmount());
        }
        return total;
    }

    double getTotalRefunds() const {
        double total = 0.0;
        for (const auto& t : transactions) {
            if (t.isRefund()) total += t.getAmount();
        }
        return total;
    }

    double getNetProfit() const {
        return getTotalRevenue() - getTotalExpenses() - getTotalRefunds();
    }

    std::vector<Transaction> getTransactionsByDate(const std::string& startDate, 
                                                  const std::string& endDate) const {
        std::vector<Transaction> result;
        for (const auto& t : transactions) {
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

    std::map<std::string, double> getDailySummary() const {
        std::map<std::string, double> summary;
        for (const auto& t : transactions) {
            std::string date = std::string(t.getTimestamp());
            if (date.length() >= 10) {
                date = date.substr(0, 10);
                if (t.isSale()) {
                    summary[date] += t.getAmount();
                } else if (t.isExpense()) {
                    summary[date] -= std::abs(t.getAmount());
                } else if (t.isRefund()) {
                    summary[date] -= t.getAmount();
                }
            }
        }
        return summary;
    }

    void displaySummary() const {
        std::cout << "\n=== SELLER FINANCIAL SUMMARY ===\n";
        std::cout << "Total Revenue: $" << getTotalRevenue() << "\n";
        std::cout << "Total Expenses: $" << getTotalExpenses() << "\n";
        std::cout << "Total Refunds: $" << getTotalRefunds() << "\n";
        std::cout << "Net Profit: $" << getNetProfit() << "\n";
    }

    void displayDetailedReport() const {
        std::cout << "\n=== DETAILED TRANSACTION REPORT ===\n";
        std::cout << "Date       | Type    | Amount  | Description\n";
        std::cout << "--------------------------------------------\n";
        
        for (const auto& t : transactions) {
            std::string date = std::string(t.getTimestamp());
            if (date.length() >= 10) date = date.substr(0, 10);
            std::string type = t.isSale() ? "SALE" : 
                             t.isExpense() ? "EXPENSE" : "REFUND";
            double amount = t.isExpense() ? -std::abs(t.getAmount()) : t.getAmount();
            
            printf("%-10s | %-7s | $%-6.2f | %s\n", 
                   date.c_str(), type.c_str(), amount, t.getDescription().c_str());
        }
    }

    void displayDailySummary() const {
        auto daily = getDailySummary();
        std::cout << "\n=== DAILY PROFIT SUMMARY ===\n";
        std::cout << "Date       | Profit/Loss\n";
        std::cout << "----------------------------\n";
        
        for (const auto& [date, profit] : daily) {
            printf("%-10s | $%-8.2f\n", date.c_str(), profit);
        }
    }

    const std::vector<Transaction>& getTransactions() const {
        return transactions;
    }
};