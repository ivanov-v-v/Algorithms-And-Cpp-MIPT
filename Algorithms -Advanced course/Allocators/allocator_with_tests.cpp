#include <iostream>
#include <ctime>
#include <chrono>
#include <cstddef>
#include <list>

#define POOLSIZE 1024

class IMemoryManager {
public:
    virtual void* allocate() = 0;
    virtual void deallocate(void*) = 0;
};

template <size_t CHUNK_SIZE>
class FixedAllocator : public IMemoryManager {
private:
    struct FreeStore {
        char storage[CHUNK_SIZE];
        FreeStore* next;
    };

    struct PoolsListNode {
        FreeStore* pool;
        PoolsListNode* next;
    };

    FreeStore* freeStoreHead;
    PoolsListNode* allocatedPoolsHead;

    inline void expandPoolSize () {

        freeStoreHead = new FreeStore[POOLSIZE];
        PoolsListNode* newlyAllocatedPool = new PoolsListNode();
        newlyAllocatedPool->pool = freeStoreHead;
        newlyAllocatedPool->next = allocatedPoolsHead;
        allocatedPoolsHead = newlyAllocatedPool;

        for (size_t i = 0; i < POOLSIZE - 1; ++i) {
            freeStoreHead[i].next = &freeStoreHead[i + 1];
        }

        freeStoreHead[POOLSIZE - 1].next = nullptr;
    }

    FixedAllocator () { // private: singleton pattern
        freeStoreHead = nullptr;
        allocatedPoolsHead = nullptr;
        expandPoolSize ();
    }
public:
    FixedAllocator(const FixedAllocator&) = delete;
    static FixedAllocator& instance() {
        static FixedAllocator instance;
        return instance;
    }
    inline virtual void* allocate () {
        if (nullptr == freeStoreHead) {
            expandPoolSize();
        }
        FreeStore* head = freeStoreHead;
        freeStoreHead = head->next;
        return static_cast<void*>(head);
    }
    inline virtual void deallocate (void* deleted) {
        FreeStore* head = static_cast<FreeStore*> (deleted);
        head->next = freeStoreHead;
        freeStoreHead = head;
    }
    virtual ~FixedAllocator () {
        while (allocatedPoolsHead) {
            PoolsListNode* nextPoolNode = allocatedPoolsHead->next;
            delete[] allocatedPoolsHead->pool;
            delete allocatedPoolsHead;
            allocatedPoolsHead = nextPoolNode;
        }
    }
};

template <class T>
class FastAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    FastAllocator() {}
    template <class U>
    FastAllocator(const FastAllocator<U>&) {}
    ~FastAllocator (){}

    template <class U>
    class rebind {
    public:
        using other = FastAllocator<U>;
    };

    pointer allocate (size_t n) {
        switch (n) {
            case 1:
                return static_cast<T*>(FixedAllocator<sizeof(T)>::instance().allocate());
            case 2:
                return static_cast<T*>(FixedAllocator<2 * sizeof(T)>::instance().allocate());
            case 4:
                return static_cast<T*>(FixedAllocator<4 * sizeof(T)>::instance().allocate());
            case 8:
                return static_cast<T*>(FixedAllocator<8 * sizeof(T)>::instance().allocate());
            case 16:
                return static_cast<T*>(FixedAllocator<16 * sizeof(T)>::instance().allocate());
            case 32:
                return static_cast<T*>(FixedAllocator<32 * sizeof(T)>::instance().allocate());
            case 64:
                return static_cast<T*>(FixedAllocator<64 * sizeof(T)>::instance().allocate());
            default:
                break;
        }
        return std::allocator<T>().allocate(n);
    }
    void deallocate(pointer ptr, size_t n) {
        switch (n) {
            case 1:
                FixedAllocator<sizeof(T)>::instance().deallocate(ptr);
                break;
            case 2:
                FixedAllocator<2 * sizeof(T)>::instance().deallocate(ptr);
                break;
            case 4:
                FixedAllocator<4 * sizeof(T)>::instance().deallocate(ptr);
                break;
            case 8:
                FixedAllocator<8 * sizeof(T)>::instance().deallocate(ptr);
                break;
            case 16:
                FixedAllocator<16 * sizeof(T)>::instance().deallocate(ptr);
                break;
            case 32:
                FixedAllocator<32 * sizeof(T)>::instance().deallocate(ptr);
                break;
            case 64:
                FixedAllocator<64 * sizeof(T)>::instance().deallocate(ptr);
                break;
            default:
                std::allocator<T>().deallocate(ptr, n);
                break;
        }
    }
    template<typename U>
    void construct(pointer ptr, U&& ref) {
        new (ptr) T(std::forward<U>(ref));
    }
    void destroy(pointer ptr) {
        ptr->~T();
    }
};

template <typename T, class Allocator = std::allocator<T>>
class List {
private:
    struct Node {
        T _value;
        Node* _prev;
        Node* _next;
        Node() {
            _prev = _next = nullptr;
        }
        Node(const T& val) {
            _value = val;
            _prev = _next = nullptr;
        }
    };

    typedef typename Allocator::template rebind<Node>::other RealAllocatorType;
    RealAllocatorType _memoryManager;

    size_t _size;
    Node* _head;
    Node* _tail;

    inline void _cleanUp() {
        while (_head) {
            _head = erase(_head);
        }
        _tail = nullptr;
    }
public:
    explicit List (const Allocator& alloc = Allocator()) {
        _memoryManager = alloc;
        _head = _tail = nullptr;
        _size = 0;
    }
    List (size_t count,  const T& value = T(), const Allocator& alloc = Allocator()) {
        _memoryManager = alloc;
        _tail = insert_after(_tail, value);
        _head = _tail;
        for (size_t i = 1; i < count; ++i) {
            _tail = insert_after(_tail, value);
        }
        _size = count;
    }
    List (const List& other) {
        _memoryManager = other._memoryManager;
        _tail = insert_after(_tail, other._head->_value);
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = insert_after(_tail, it->_value);
        }
        _size = other._size;
    }
    List (List&& other) {
        _memoryManager = other._memoryManager;
        _tail = insert_after(_tail, std::move(other._head->_value));
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = insert_after(_tail, std::move(it->_value));
        }
        _size = std::move(other._size);
    }
    List& operator =(const List& other) {
        _cleanUp();
        _memoryManager = other._memoryManager;
        _tail = insert_after(_tail, other._head->_value);
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = insert_after(_tail, it->_value);
        }
        _size = other._size;
        return *this;
    }
    List& operator =(List&& other) {
        _cleanUp();
        _memoryManager = other._memoryManager;
        _tail = insert_after(_tail, std::move(other._head->_value));
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = insert_after(_tail, std::move(it->_value));
        }
        _size = std::move(other._size);
        return *this;
    }
    ~List () {
        _cleanUp();
    }
    size_t size () const {
        return _size;
    }

    template <typename U>
    Node* insert_after (Node *curr, U&& val) {
        Node* nextNode = _memoryManager.allocate(1);
        nextNode->_value = std::forward<U>(val);
        nextNode->_prev = nextNode->_next = nullptr;

        if (curr) {
            nextNode->_prev = curr;
            nextNode->_next = curr->_next;
            if (curr->_next) {
                curr->_next->_prev = nextNode;
            }
            curr->_next = nextNode;
        }
        ++_size;
        return nextNode;
    }

    template <typename U>
    Node* insert_before (Node* curr, U&& val) {
        Node* prevNode = _memoryManager.allocate(1);
        prevNode->_value = std::forward<U>(val);
        prevNode->_prev = prevNode->_next = nullptr;

        if (curr) {
            prevNode->_next = curr;
            prevNode->_prev = curr->_prev;
            if (curr->_prev) {
                curr->_prev->_next = prevNode;
            }
            curr->_prev = prevNode;
        }
        ++_size;
        return prevNode;
    }

    Node* erase (Node *curr) {
        Node* neighborNode;
        if (!curr || (!curr->_prev && !curr->_next)) {
            neighborNode = nullptr;
        } else if (!curr->_next) {
            neighborNode = curr->_prev;
            neighborNode->_next = nullptr;
        } else if (!curr->_prev) {
            neighborNode = curr->_next;
            neighborNode->_prev = nullptr;
        } else {
            neighborNode = curr->_next;
            curr->_next->_prev = curr->_prev;
            curr->_prev->_next = curr->_next;
        }
        _memoryManager.deallocate(curr, 1);
        if (_size) {
            --_size;
        }
        return neighborNode;
    }

    template <typename U>
    Node* push_back(U&& val) {
        _tail = insert_after(_tail, std::forward<U>(val));
        if (!_head) {
            _head = _tail;
        }
        return _tail;
    }

    template <typename U>
    Node* push_front(U&& val) {
        _head = insert_before(_head, std::forward<U>(val));
        if (!_tail) {
            _tail = _head;
        }
        return _head;
    }

    Node* pop_back () {
        _tail = erase(_tail);
        if (!_tail) {
            _head = nullptr;
        }
        return _tail;
    }
    Node* pop_front () {
        _head = erase(_head);
        if (!_head) {
            _tail = nullptr;
        }
        return _head;
    }

    void display () const {
        Node* curr = _head;
        while (curr) {
            std::cout << curr->_value << " ";
            curr = curr->_next;
        }
        std::cout << std::endl;
    }
};

template <class List>
void test(std::string comment, List l) {
    std::cout << comment;
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1e7; ++i) {
        int cmd = rand() % 100;
        if (!l.size() || cmd < 60) {
            if (rand() & 1) {
                l.push_front(rand());
            } else {
                l.push_back(rand());
            }
        } else if (cmd < 80) {
            l.pop_front();
        } else {
            l.pop_back();
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << std::endl;
}

#define TEST_CASES 1

int main() {

    for (size_t i = 0; i < TEST_CASES; ++i) {
        test("my list, std::alloc: ", List<int>());
        test("my list, my alloc: ", List<int, FastAllocator<int>>());
        test("std::list, std::alloc: ", std::list<int>());
        test("std::list, my alloc: ", std::list<int, FastAllocator<int>>());
    }
    return 0;
}
