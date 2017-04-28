#define POOLSIZE 1024

class IMemoryManager {
public:
    virtual void* allocate(size_t) = 0;
    virtual void deallocate(void*, size_t) = 0;
};

template <size_t CHUNK_SIZE>
class FixedAllocator : public IMemoryManager {
private:
    struct FreeStore {
        char storage[CHUNK_SIZE];
        FreeStore* next;
    };

    FreeStore* freeStoreHead;
    vector<FreeStore*> pools;
//    size_t last_block_size = 1;
//    const size_t MAX_SIZE = 512;

    inline void expandPoolSize () {
//        if (last_block_size < MAX_SIZE) {
//            last_block_size *= 2;
//        }

        freeStoreHead = new FreeStore[POOLSIZE];
        pools.push_back(freeStoreHead);

        for (size_t i = 0; i < POOLSIZE - 1; ++i) {
            freeStoreHead[i].next = &freeStoreHead[i + 1];
        }

        freeStoreHead[POOLSIZE - 1].next = nullptr;
    }

    FixedAllocator () { // private: singleton pattern
        freeStoreHead = nullptr;
        expandPoolSize ();
    }
public:
    FixedAllocator(const FixedAllocator&) = delete;
    static FixedAllocator& instance() {
        static FixedAllocator instance;
        return instance;
    }
    inline virtual void* allocate (size_t size) {
        if (nullptr == freeStoreHead) {
            expandPoolSize();
        }
        FreeStore* head = freeStoreHead;
        freeStoreHead = head->next;
        return static_cast<void*>(head);
    }
    inline virtual void deallocate (void* deleted, size_t n) {
        FreeStore* head = static_cast<FreeStore*> (deleted);
        head->next = freeStoreHead;
        freeStoreHead = head;
    }
    virtual ~FixedAllocator () {
        for (auto ptr : pools) {
            delete [] ptr;
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
    FastAllocator(const FastAllocator<U>& another) {}
    ~FastAllocator (){}

    template <class U>
    class rebind {
    public:
        using other = FastAllocator<U>;
    };

    pointer allocate (size_t n) {
        // как это же написать посимпатичнее?
        switch (n) {
            case 1:
                return static_cast<T*>(FixedAllocator<sizeof(T)>::instance().allocate(1));
            case 2:
                return static_cast<T*>(FixedAllocator<2 * sizeof(T)>::instance().allocate(1));
            case 4:
                return static_cast<T*>(FixedAllocator<4 * sizeof(T)>::instance().allocate(1));
            case 8:
                return static_cast<T*>(FixedAllocator<8 * sizeof(T)>::instance().allocate(1));
            case 16:
                return static_cast<T*>(FixedAllocator<16 * sizeof(T)>::instance().allocate(1));
            case 32:
                return static_cast<T*>(FixedAllocator<32 * sizeof(T)>::instance().allocate(1));
            case 64:
                return static_cast<T*>(FixedAllocator<64 * sizeof(T)>::instance().allocate(1));
            default:
                break;
        }
        return std::allocator<T>().allocate(n);
    }
    void deallocate(pointer ptr, size_t n) {
        switch (n) {
            case 1:
                FixedAllocator<sizeof(T)>::instance().deallocate(ptr, 1);
                break;
            case 2:
                FixedAllocator<2 * sizeof(T)>::instance().deallocate(ptr, 1);
                break;
            case 4:
                FixedAllocator<4 * sizeof(T)>::instance().deallocate(ptr, 1);
                break;
            case 8:
                FixedAllocator<8 * sizeof(T)>::instance().deallocate(ptr, 1);
                break;
            case 16:
                FixedAllocator<16 * sizeof(T)>::instance().deallocate(ptr, 1);
                break;
            case 32:
                FixedAllocator<32 * sizeof(T)>::instance().deallocate(ptr, 1);
                break;
            case 64:
                FixedAllocator<64 * sizeof(T)>::instance().deallocate(ptr, 1);
                break;
            default:
                std::allocator<T>().deallocate(ptr, n);
                break;
        }
    }
    void construct(pointer ptr, const_reference ref) {
        new (ptr) T(ref);
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

    Node* _insertAfter(Node* curr, const T& val) {
        Node* nextNode = _memoryManager.allocate(1);
        nextNode->_value = val;
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
    Node* _insertAfter(Node* curr, T&& val) {
        Node* nextNode = _memoryManager.allocate(1);
        nextNode->_value = std::move(val);
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

    Node* _insertBefore(Node* curr, const T& val) {
        Node* prevNode = _memoryManager.allocate(1);
        prevNode->_value = val;
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
    Node* _insertBefore(Node* curr, T&& val) {
        Node* prevNode = _memoryManager.allocate(1);
        prevNode->_value = std::move(val);
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

    Node* _erase(Node* curr) {
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

    size_t _size;
    Node* _head;
    Node* _tail;

    inline void _cleanUp() {
        while (_head) {
            _head = _erase(_head);
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
        _tail = _insertAfter(_tail, value);
        _head = _tail;
        for (size_t i = 1; i < count; ++i) {
            _tail = _insertAfter(_tail, value);
        }
        _size = count;
    }
    List (const List& other) {
        _memoryManager = other._memoryManager;
        _tail = _insertAfter(_tail, other._head->_value);
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = _insertAfter(_tail, it->_value);
        }
        _size = other._size;
    }
    List (List&& other) {
        _memoryManager = other._memoryManager;
        _tail = _insertAfter(_tail, std::move(other._head->_value));
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = _insertAfter(_tail, std::move(it->_value));
        }
        _size = std::move(other._size);
    }
    List& operator =(const List& other) {
        _cleanUp();
        _memoryManager = other._memoryManager;
        _tail = _insertAfter(_tail, other._head->_value);
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = _insertAfter(_tail, it->_value);
        }
        _size = other._size;
        return *this;
    }
    List& operator =(List&& other) {
        _cleanUp();
        _memoryManager = other._memoryManager;
        _tail = _insertAfter(_tail, std::move(other._head->_value));
        _head = _tail;
        for (Node* it = other._head->_next; it; it = it->_next) {
            _tail = _insertAfter(_tail, std::move(it->_value));
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

    Node* push_back (const T& val) {
        _tail = _insertAfter(_tail, val);
        if (!_head) {
            _head = _tail;
        }
        return _tail;
    }
    Node* push_back(T&& val) {
        _tail = _insertAfter(_tail, std::move(val));
        if (!_head) {
            _head = _tail;
        }
        return _tail;
    }

    Node* push_front (const T& val) {
        _head = _insertBefore(_head, val);
        if (!_tail) {
            _tail = _head;
        }
        return _head;
    }
    Node* push_front(T&& val) {
        _head = _insertBefore(_head, std::move(val));
        if (!_tail) {
            _tail = _head;
        }
        return _head;
    }

    Node* pop_back () {
        _tail = _erase(_tail);
        if (!_tail) {
            _head = nullptr;
        }
        return _tail;
    }
    Node* pop_front () {
        _head = _erase(_head);
        if (!_head) {
            _tail = nullptr;
        }
        return _head;
    }

    void display () const {
        Node* curr = _head;
        while (curr) {
            cout << curr->_value << " ";
            curr = curr->_next;
        }
        cout << endl;
    }
};

template <class List>
void test(std::string comment, List l) {
    std::cout << comment;
    auto start_time = chrono::high_resolution_clock::now();
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
    auto end_time = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() << endl;
}

#define TEST_CASES 1

int main() {
//    ios::sync_with_stdio(false);
//    std::cin.tie(0);
//    freopen("in", "r", stdin);
//    freopen("out", "w", stdout);

    for (size_t i = 0; i < TEST_CASES; ++i) {
        test("my list, std::alloc: ", List<int>());
        test("my list, my alloc: ", List<int, FastAllocator<int>>());
        test("std::list, std::alloc: ", std::list<int>());
        test("std::list, my alloc: ", std::list<int, FastAllocator<int>>());
    }

//    List<int, FastAllocator<int>> myList;
//    string cmd;
//    cin >> cmd;
//    while (cmd != "exit") {
//        if (cmd == "L") {
//            int val;
//            cin >> val;
//            myList.push_front(val);
//        } else if (cmd == "R") {
//            int val;
//            cin >> val;
//            myList.push_back(val);
//        } else if (cmd == "-L") {
//            if (myList.size()) {
//                myList.pop_front();
//            } else {
//                cout << "Empty list" << endl;
//            }
//        } else if (cmd == "-R") {
//            if (myList.size()) {
//                myList.pop_back();
//            } else {
//                cout << "Empty list" << endl;
//            }
//        } else {
//            cout << "invalid command" << endl;
//        }
//        myList.display();
//        cin >> cmd;
//    }
    return 0;
}
