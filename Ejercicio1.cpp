#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Account {
    std::string id_;
    double balance_;
    friend class Transaction;   // única clase autorizada a cambiar el saldo
public:
    Account(std::string id, double balance) : id_(std::move(id)), balance_(balance) {}
    const std::string& getId() const { return id_; }
    double getBalance() const { return balance_; }
};

class Transaction {
public:
    enum class Type { Deposit, Withdraw };

    Transaction(Type type, Account* account, double amount)
        : type_(type), account_(account), amount_(amount) {}

    bool execute() {
        if (type_ == Type::Deposit) {
            account_->balance_ += amount_;
            return true;
        }
        if (account_->balance_ < amount_) return false;
        account_->balance_ -= amount_;
        return true;
    }

private:
    Type type_;
    Account* account_;
    double amount_;
};

class TransactionManager {
    std::mutex mutex_;
    std::vector<Transaction> history_;   // registro de operaciones exitosas

    TransactionManager() = default;

    bool run(Transaction t) {
        std::lock_guard<std::mutex> lock(mutex_);   // se libera solo al salir
        bool ok = t.execute();
        if (ok) history_.push_back(t);
        return ok;
    }

public:
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;

    static TransactionManager& getInstance() {
        static TransactionManager instance;
        return instance;
    }

    bool deposit(Account* account, double amount) {
        return run(Transaction(Transaction::Type::Deposit, account, amount));
    }

    bool withdraw(Account* account, double amount) {
        return run(Transaction(Transaction::Type::Withdraw, account, amount));
    }

    size_t transactionCount() {
        std::lock_guard<std::mutex> lock(mutex_);
        return history_.size();
    }
};

int main() {
    auto& manager = TransactionManager::getInstance();

    Account account("A1", 1000);

    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            manager.deposit(&account, 100);
            manager.withdraw(&account, 50);
        });
    }

    for (auto& t : threads) t.join();

    std::cout << "Balance final: "
              << account.getBalance()
              << std::endl;

    std::cout << "Direccion manager: "
              << &TransactionManager::getInstance()
              << std::endl;
}
