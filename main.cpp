#include "lock_manager.hpp"
#include <fstream>
#include <sstream>
#include <set>

int main() {
    LockManager lm;

    std::ifstream fin("input.txt");
    std::string line;
    std::set<int> registered;

    while (getline(fin, line)) {
        std::stringstream ss(line);
        std::string tid_str, op_str, item;
        ss >> tid_str >> op_str >> item;

        int tid = stoi(tid_str.substr(1)); // skip 'T'

        if (!registered.count(tid)) {
            lm.registerTransaction(tid);
            registered.insert(tid);
        }

        OperationType type = (op_str == "R" ? READ : WRITE);
        lm.processOperation({ tid, type, item });
    }

    lm.retryAborted();

    return 0;
}
