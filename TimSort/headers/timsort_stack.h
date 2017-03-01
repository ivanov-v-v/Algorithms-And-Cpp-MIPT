#ifndef TIMSORT_TIMSORT_STACK_H
#define TIMSORT_TIMSORT_STACK_H
#include <vector>
#include <cstdint>
using std::vector;

namespace timsort {
//-------------------------------------------------------------------------------------
    template<typename T>
    class stack {
    private:
        vector<T> storage;
    public:
        uint32_t size () const {
            return storage.size();
        }
        T pop() {
            if (storage.empty()) {
                throw std::logic_error("POP FROM EMPTY STACK");
            }
            T value = storage.back();
            storage.pop_back();
            return value;
        }
        void push(const T &entry) {
            storage.push_back(entry);
        }
    };
//-------------------------------------------------------------------------------------
}
#endif //TIMSORT_TIMSORT_STACK_H