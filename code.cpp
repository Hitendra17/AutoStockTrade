#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

// Transaction class to record each trade
class Transaction {
public:
    string stock;
    int quantity;
    double price;
    string type; // buy or sell
    int timestamp;

    Transaction(string stock, int quantity, double price, string type, int timestamp)
        : stock(stock), quantity(quantity), price(price), type(type), timestamp(timestamp) {}

    void printTransaction() const {
        cout << "Timestamp: " << timestamp << " | " << type << " | "
             << stock << ": " << quantity << " shares at $" << fixed << setprecision(2) << price << endl;
    }
};

// StockOrder class for handling buy/sell orders
class StockOrder {
public:
    string type; // buy or sell
    string stock;
    int quantity;
    double price;
    int timestamp;
    string orderType; // market, limit, or stop

    StockOrder(string type, string stock, int quantity, double price, int timestamp, string orderType)
        : type(type), stock(stock), quantity(quantity), price(price), timestamp(timestamp), orderType(orderType) {}

    bool operator<(const StockOrder& other) const {
        // Orders are compared by their timestamp
        return timestamp < other.timestamp;
    }
};

// Portfolio class to manage user's stocks and balance
class Portfolio {
private:
    map<string, int> holdings; // Stock holdings
    double balance;

public:
    Portfolio() : balance(10000.0) {} // Initialize with a balance of $10,000

    bool canBuy(double amount) const {
        return balance >= amount;
    }

    bool canSell(const string& stock, int quantity) const {
        auto it = holdings.find(stock);
        return it != holdings.end() && it->second >= quantity;
    }

    void updateHoldings(const string& stock, int quantity) {
        holdings[stock] += quantity;
    }

    void adjustBalance(double amount) {
        balance += amount;
    }

    void printPortfolio() const {
        cout << "\nPortfolio:" << endl;
        for (const auto& pair : holdings) {
            cout << pair.first << ": " << pair.second << " shares" << endl;
        }
        cout << "Balance: $" << fixed << setprecision(2) << balance << endl;
    }

    void printPerformance() const {
        cout << "Performance metrics are not implemented yet." << endl;
    }
};

// Market class to handle stock price updates
class Market {
private:
    map<string, double> currentPrices; // Current prices for each stock
    vector<string> stockSymbols;

public:
    Market(const vector<string>& symbols) : stockSymbols(symbols) {
        // Initialize prices
        for (const auto& symbol : stockSymbols) {
            currentPrices[symbol] = 100.0; // Example initial price
        }
    }

    void updatePrices() {
        // Simulate price changes
        for (auto& pair : currentPrices) {
            pair.second += (rand() % 100 - 50) / 10.0; // Random fluctuation
        }
    }

    double getPrice(const string& stock) const {
        auto it = currentPrices.find(stock);
        return it != currentPrices.end() ? it->second : 0.0;
    }

    // Getter for currentPrices
    const map<string, double>& getCurrentPrices() const {
        return currentPrices;
    }

    void printCurrentPrices() const {
        cout << "\nCurrent Prices:" << endl;
        for (const auto& pair : currentPrices) {
            cout << "Stock: " << pair.first << " - Price: $" << fixed << setprecision(2) << pair.second << endl;
        }
        cout << endl;
    }
};

// User class to manage user information and authentication
class User {
private:
    string username;
    string passwordHash; // Simple hashing for password storage

public:
    Portfolio portfolio;

    User() : username(""), passwordHash("") {} // Default constructor

    User(string username, string password) : username(username), passwordHash(password) {}

    bool checkPassword(const string& password) const {
        return passwordHash == password; // Simple password check
    }

    string getUsername() const {
        return username;
    }

    Portfolio& getPortfolio() {
        return portfolio;
    }
};

// TradingSystem class to handle overall trading operations
class TradingSystem {
private:
    Market market;
    map<string, User> users;
    User* currentUser;
    priority_queue<StockOrder> buyOrders;
    priority_queue<StockOrder> sellOrders;
    vector<Transaction> transactionHistory;
    int timestamp;

public:
    TradingSystem(Market market) : market(market), currentUser(nullptr), timestamp(0) {}

    void addUser(const string& username, const string& password) {
        if (users.find(username) != users.end()) {
            cout << "Username already exists!" << endl;
            return;
        }
        // Create a User object and add it to the map
        users.emplace(username, User(username, password));
        cout << "User registered successfully!" << endl;
    }

    bool authenticateUser(const string& username, const string& password) {
        auto it = users.find(username);
        if (it != users.end() && it->second.checkPassword(password)) {
            currentUser = &(it->second);
            cout << "Login successful!" << endl;
            return true;
        }
        cout << "Invalid username or password!" << endl;
        return false;
    }

    void addBuyOrder(const string& stock, int quantity, double price, string orderType = "market") {
        if (!currentUser) {
            cout << "Please log in first!" << endl;
            return;
        }
        double totalCost = price * quantity;
        if (!currentUser->getPortfolio().canBuy(totalCost)) {
            cout << "Insufficient funds for this buy order!" << endl;
            return;
        }
        buyOrders.push(StockOrder("buy", stock, quantity, price, timestamp++, orderType));
        cout << "Added Buy Order: " << quantity << " shares of " << stock << " at $" << price << endl;
    }

    void addSellOrder(const string& stock, int quantity, double price, string orderType = "market") {
        if (!currentUser) {
            cout << "Please log in first!" << endl;
            return;
        }
        if (!currentUser->getPortfolio().canSell(stock, quantity)) {
            cout << "Insufficient shares for this sell order!" << endl;
            return;
        }
        sellOrders.push(StockOrder("sell", stock, quantity, price, timestamp++, orderType));
        cout << "Added Sell Order: " << quantity << " shares of " << stock << " at $" << price << endl;
    }

    void executeOrders() {
        market.updatePrices();              // Simulate market price update
        market.printCurrentPrices();        // Print current prices for reference

        // Process buy orders
        while (!buyOrders.empty()) {
            StockOrder buyOrder = buyOrders.top();
            double marketPrice = market.getPrice(buyOrder.stock);
            if (buyOrder.orderType == "market" || (buyOrder.orderType == "limit" && buyOrder.price >= marketPrice)) {
                // Buy order matches current market price
                currentUser->getPortfolio().updateHoldings(buyOrder.stock, buyOrder.quantity);
                currentUser->getPortfolio().adjustBalance(-marketPrice * buyOrder.quantity);
                transactionHistory.push_back(Transaction(buyOrder.stock, buyOrder.quantity, marketPrice, "buy", buyOrder.timestamp));
                buyOrders.pop();
            } else {
                break; // No further matching buy orders at current market price
            }
        }

        // Process sell orders
        while (!sellOrders.empty()) {
            StockOrder sellOrder = sellOrders.top();
            double marketPrice = market.getPrice(sellOrder.stock);
            if (sellOrder.orderType == "market" || (sellOrder.orderType == "limit" && sellOrder.price <= marketPrice)) {
                // Sell order matches current market price
                currentUser->getPortfolio().updateHoldings(sellOrder.stock, -sellOrder.quantity);
                currentUser->getPortfolio().adjustBalance(marketPrice * sellOrder.quantity);
                transactionHistory.push_back(Transaction(sellOrder.stock, sellOrder.quantity, marketPrice, "sell", sellOrder.timestamp));
                sellOrders.pop();
            } else {
                break; // No further matching sell orders at current market price
            }
        }

        // Output transaction history
        cout << "\nExecuted transactions:" << endl;
        for (const auto& txn : transactionHistory) {
            txn.printTransaction();
        }
        cout << endl;
    }

    void printTransactionHistory() {
        cout << "\nTransaction History:" << endl;
        for (const auto& txn : transactionHistory) {
            txn.printTransaction();
        }
    }

    void displayPortfolio() {
        if (!currentUser) {
            cout << "Please log in first!" << endl;
            return;
        }
        currentUser->getPortfolio().printPortfolio();
    }

    void generateReport() {
        cout << "Generating report is not yet implemented." << endl;
    }

    void backtestStrategy(const string& stock, const vector<double>& historicalPrices) {
        // Simple backtesting strategy (e.g., moving average)
        if (historicalPrices.size() < 2) {
            cout << "Not enough data for backtesting." << endl;
            return;
        }

        double totalGain = 0.0;
        double previousPrice = historicalPrices[0];

        for (size_t i = 1; i < historicalPrices.size(); ++i) {
            double currentPrice = historicalPrices[i];
            totalGain += currentPrice - previousPrice;
            previousPrice = currentPrice;
        }

        cout << "Backtesting results for " << stock << ":" << endl;
        cout << "Total Gain: $" << fixed << setprecision(2) << totalGain << endl;
    }

    void importUserData(const string& filename) {
        ifstream file(filename);
        if (!file) {
            cout << "Error opening file for import!" << endl;
            return;
        }

        string username, password;
        while (file >> username >> password) {
            addUser(username, password);
        }
        file.close();
        cout << "User data imported successfully!" << endl;
    }

    void exportUserData(const string& filename) {
        ofstream file(filename);
        if (!file) {
            cout << "Error opening file for export!" << endl;
            return;
        }

        for (const auto& userPair : users) {
            file << userPair.first << " " << userPair.second.checkPassword("") << endl; // Adjust as needed
        }
        file.close();
        cout << "User data exported successfully!" << endl;
    }

    void userInteraction() {
        string command;
        while (true) {
            cout << "Enter command (register, login, buy, sell, portfolio, report, backtest, import, export, quit): ";
            cin >> command;
            if (command == "register") {
                string username, password;
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;
                addUser(username, password);
            } else if (command == "login") {
                string username, password;
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;
                authenticateUser(username, password);
            } else if (command == "buy") {
                string stock;
                int quantity;
                double price;
                cout << "Enter stock symbol: ";
                cin >> stock;
                cout << "Enter quantity: ";
                cin >> quantity;
                cout << "Enter price: ";
                cin >> price;
                addBuyOrder(stock, quantity, price);
            } else if (command == "sell") {
                string stock;
                int quantity;
                double price;
                cout << "Enter stock symbol: ";
                cin >> stock;
                cout << "Enter quantity: ";
                cin >> quantity;
                cout << "Enter price: ";
                cin >> price;
                addSellOrder(stock, quantity, price);
            } else if (command == "portfolio") {
                displayPortfolio();
            } else if (command == "report") {
                generateReport();
            } else if (command == "backtest") {
                string stock;
                int numPrices;
                cout << "Enter stock symbol: ";
                cin >> stock;
                cout << "Enter number of historical prices: ";
                cin >> numPrices;
                vector<double> prices(numPrices);
                cout << "Enter historical prices: ";
                for (int i = 0; i < numPrices; ++i) {
                    cin >> prices[i];
                }
                backtestStrategy(stock, prices);
            } else if (command == "import") {
                string filename;
                cout << "Enter filename: ";
                cin >> filename;
                importUserData(filename);
            } else if (command == "export") {
                string filename;
                cout << "Enter filename: ";
                cin >> filename;
                exportUserData(filename);
            } else if (command == "quit") {
                break;
            } else {
                cout << "Unknown command!" << endl;
            }
            executeOrders();
        }
    }
};

int main() {
    vector<string> stocks = {"AAPL", "GOOGL", "MSFT"};
    Market market(stocks);
    TradingSystem tradingSystem(market);

    tradingSystem.userInteraction();

    return 0;
}
