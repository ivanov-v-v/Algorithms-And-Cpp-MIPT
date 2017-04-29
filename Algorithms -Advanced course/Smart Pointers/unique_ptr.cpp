template <typename T>
struct defaultDeleter {
    void operator() (T* ptr) noexcept {
        delete ptr;
    }
};

template <typename T, class Deleter = defaultDeleter<T>>
class UniquePtr {
public:
    using val_t = T;
    using ref_t = T&;
    using ptr_t = T*;

    // Constructors that make an empty unique_ptr
    constexpr UniquePtr () noexcept
            : _ptr (nullptr), _deleter() {}
    constexpr UniquePtr (std::nullptr_t) noexcept
            : _ptr(nullptr), _deleter() {}
    // Constructor that makes a unique_ptr from raw pointer
    explicit UniquePtr (ptr_t ptr) : _ptr(ptr), _deleter() {}
    UniquePtr (UniquePtr&& other) noexcept
            : _ptr(other._ptr), _deleter(std::move(other._deleter)) {
        other._ptr = nullptr;
    }
    // Modification (to allow upcasting in hierarchy)
    template <class T2, class D2>
    UniquePtr (UniquePtr<T2, D2>&& other) noexcept
            : _ptr(other._ptr), _deleter(std::move(other._deleter)) {
        other._ptr = nullptr;
    }

    UniquePtr (ptr_t ptr, typename std::conditional <
            std::is_reference<Deleter>::value,
            Deleter,
            typename std::add_lvalue_reference<const Deleter>::type>::type
        deleter) : _ptr(ptr), _deleter(deleter) {}

    UniquePtr (ptr_t ptr, typename std::remove_reference<Deleter>::type&& deleter)
            : _ptr(ptr), _deleter(deleter) {}
    // Move-assignment operator
    UniquePtr& operator= (UniquePtr&& other) noexcept {
        swap(other);
        return *this;
    }
    // Unique_ptr is not copy-assignable and not copy-constructible
    UniquePtr (const UniquePtr& other) = delete;
    UniquePtr& operator= (const UniquePtr& other) = delete;
    // Destructor which invokes custom deleter function
    ~UniquePtr () {
        _deleter(_ptr);
    }
    // Safe way to swap unique_ptrs
    void swap(UniquePtr& other) noexcept {
        std::swap(_ptr, other._ptr);
        std::swap(_deleter, other._deleter);
    }
    // Check whether unique_ptr owns any object
    explicit operator bool() const noexcept {
        return _ptr != nullptr;
    }
    ptr_t get() const noexcept {
        return _ptr;
    }
    // Overload of dereferencing operators so unique_ptr
    // resembles a raw pointer
    ref_t operator* () const noexcept {
        return *_ptr;
    }
    ptr_t operator-> () const noexcept {
        return _ptr;
    }
    // Renounce the ownership
    ptr_t release () noexcept {
        ptr_t storedPtr = _ptr;
        _ptr = nullptr;
        return storedPtr;
    }
    // Renounce old and accept new ownership
    void reset (nullptr_t nptr) noexcept {
        _deleter(_ptr);
        _ptr = nullptr;
    }
    void reset (ptr_t newPtr = ptr_t()) noexcept {
        ptr_t oldPtr = _ptr;
        _ptr = newPtr;
        _deleter(oldPtr);
    }
    // Access to deleter object
    Deleter& getDeleter() noexcept {
        return _deleter;
    }
    const Deleter& getDeleter() const noexcept  {
        return _deleter;
    }
private:
    ptr_t _ptr;
    Deleter _deleter;
};

// wrapper
template <typename T, typename D>
void swap (UniquePtr<T, D>& p1, UniquePtr<T, D>& p2) noexcept {
    p1.swap(p2);
}

