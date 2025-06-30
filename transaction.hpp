#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <string>
#include <set>

enum OperationType { READ, WRITE };

struct Operation {
    int transaction_id;
    OperationType op_type;
    std::string data_item;
};

struct Transaction {
    int id;
    int timestamp;
    bool active = true;
    bool aborted = false;
    std::set<std::string> locked_items;

    Transaction(int id, int timestamp) : id(id), timestamp(timestamp) {}
};

#endif // TRANSACTION_HPP
