#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

#include "transaction.hpp"
#include <unordered_map>
#include <queue>
#include <iostream>

struct Lock {
    int holder_id;
    OperationType type;
};

class LockManager {
    std::unordered_map<std::string, Lock> lock_table;
    std::unordered_map<int, Transaction*> transactions;
    std::queue<Operation> waiting_queue;
    std::queue<Operation> retry_queue;
    int current_time = 0;

public:
    void registerTransaction(int id) {
        transactions[id] = new Transaction(id, current_time++);
    }

    void processOperation(const Operation& op) {
        Transaction* txn = transactions[op.transaction_id];
        if (!txn->active) {
            std::cout << "T" << txn->id << " is aborted. Skipping.\n";
            return;
        }

        std::string item = op.data_item;

        // No lock exists
        if (lock_table.find(item) == lock_table.end()) {
            lock_table[item] = { txn->id, op.op_type };
            txn->locked_items.insert(item);
            std::cout << "T" << txn->id << " acquired "
                      << (op.op_type == READ ? "Read" : "Write")
                      << " lock on " << item << "\n";
        } else {
            Lock& existing = lock_table[item];
            if (existing.holder_id == txn->id) {
                std::cout << "T" << txn->id << " already holds the lock on " << item << "\n";
                return;
            }

            Transaction* holder = transactions[existing.holder_id];

            if (txn->timestamp < holder->timestamp) {
                std::cout << "T" << txn->id << " is older than T" << holder->id
                          << " → waits on " << item << "\n";
                waiting_queue.push(op);
            } else {
                std::cout << "T" << txn->id << " is younger than T" << holder->id
                          << " → dies and rolls back\n";
                retry_queue.push(op);
                abortTransaction(txn->id);
            }
        }

        printLockTable();
    }

    void abortTransaction(int id) {
        Transaction* txn = transactions[id];
        txn->active = false;
        txn->aborted = true;
        for (const std::string& item : txn->locked_items) {
            lock_table.erase(item);
        }
        txn->locked_items.clear();
    }

    void retryAborted() {
        int retry_count = retry_queue.size();
        std::cout << "\n=== Retrying Aborted Transactions ===\n";

        for (int i = 0; i < retry_count; ++i) {
            Operation op = retry_queue.front();
            retry_queue.pop();

            Transaction* txn = transactions[op.transaction_id];
            if (!txn->active) {
                txn->active = true;
                txn->aborted = false;
                txn->timestamp = current_time++;
                std::cout << "Restarting T" << txn->id
                          << " with new timestamp " << txn->timestamp << "\n";
            }

            processOperation(op);
        }
    }

    void printLockTable() {
        std::cout << "\n--- Lock Table ---\n";
        if (lock_table.empty()) {
            std::cout << "(empty)\n";
        } else {
            for (const auto& [item, lock] : lock_table) {
                std::cout << "Data Item: " << item
                          << ", Held by T" << lock.holder_id
                          << ", Type: " << (lock.type == READ ? "Read" : "Write") << "\n";
            }
        }
        std::cout << "------------------\n\n";
    }

    ~LockManager() {
        for (auto& [_, t] : transactions) delete t;
    }
};

#endif // LOCK_MANAGER_HPP
