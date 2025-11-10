#pragma once
#include <string>

using UserId = int;
using ProductId = int;
using OrderId = int;
using TransactionId = int;

constexpr const char* PRODUCT_FILE = "data/products.dat";
constexpr const char* USER_FILE = "data/users.dat";
constexpr const char* ORDER_FILE = "data/orders.dat";
constexpr const char* TRANSACTION_FILE = "data/transactions.dat";
constexpr const char* CART_FILE_PREFIX = "data/cart_";

constexpr int MAX_PRODUCTS = 1000;
constexpr int MAX_USERS = 500;
constexpr int MAX_CART_ITEMS = 50;
constexpr int DATE_STR_LEN = 20;

enum class UserType { CUSTOMER, SELLER, ADMIN };
enum class TransactionType { SALE, REFUND, EXPENSE, DEPOSIT };

struct CartItem 
{
    ProductId productId;
    int quantity;
};