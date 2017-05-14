#include <cstddef>

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
            : _ptr(std::move(other._ptr)), _deleter(std::move(other._deleter)) {
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
        _deleter(_ptr);
        swap(other);
        other._ptr = nullptr;
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


struct ControlBlock {
    int reference_count;
    int weak_count;
    ControlBlock() {
        reference_count = 0;
        weak_count = 0;
    }
};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr {
public:
    using val_t = T;
    using ref_t = T&;
    using ptr_t = T*;

    constexpr SharedPtr () noexcept
        : _ptr(nullptr), _cblock(nullptr) {}
    constexpr SharedPtr (std::nullptr_t) noexcept
        : _ptr(nullptr), _cblock(nullptr) {}
    explicit SharedPtr (ptr_t ptr)
        : _ptr(ptr), _cblock(new ControlBlock()) {
        _increment_counter();
    }
    SharedPtr (const SharedPtr& other)
        : _ptr(other._ptr), _cblock(other._cblock) {
        _increment_counter();
    }
    SharedPtr (SharedPtr&& other)
        : _ptr(other._ptr), _cblock(other._cblock) {
        other._ptr = nullptr;
        other._cblock = nullptr;
    }
    SharedPtr (const WeakPtr<T>& other) noexcept
        : _ptr(other._ptr), _cblock(other._cblock) {
        if (!_cblock) {
            _cblock = new ControlBlock();
        }
        _increment_counter();
    }

    SharedPtr& operator= (const SharedPtr& other) noexcept {
        _decrement_counter();
        _clean_up();
        _ptr = other._ptr;
        _cblock = other._cblock;
        _increment_counter();
        return *this;
    }
    SharedPtr& operator= (SharedPtr&& other) noexcept {
        _decrement_counter();
        _clean_up();
        swap(other);
        other._ptr = nullptr;
        other._cblock = nullptr;
        return *this;
    }

    ~SharedPtr () {
        _decrement_counter();
        _clean_up();
    }

    void swap(SharedPtr& other) noexcept {
        std::swap(_ptr, other._ptr);
        std::swap(_cblock, other._cblock);
    }

    ptr_t get () const noexcept {
        return _ptr;
    }
    ref_t operator* () const noexcept { // why noexcept here?
        return *_ptr;
    }
    ptr_t operator-> () const noexcept {
        return _ptr;
    }

    int use_count () const noexcept {
        return _cblock ? _cblock->reference_count : 0;
    }

    void reset (nullptr_t nptr) noexcept {
        _decrement_counter();
        // если это последний умный указатель,
        // ссылавшийся на данный объект,
        // то удалить сам объект и управляющий им блок
        _clean_up();
        _ptr = nullptr;
    }
    void reset (ptr_t newPtr = ptr_t()) noexcept {
        if (_ptr != newPtr) {
            _decrement_counter();
            // если это последний умный указатель,
            // ссылавшийся на данный объект,
            // то удалить сам объект и управляющий им блок
            _clean_up();
            _ptr = newPtr;
            _cblock = new ControlBlock();
            _increment_counter();
        }
    }

    template <typename>
    friend class WeakPtr;
private:
    ptr_t _ptr;
    ControlBlock* _cblock;

    void _clean_up() noexcept {
        if (!use_count()) {
            delete _ptr;
            if (_cblock && !_cblock->weak_count) {
                delete _cblock;
            }
        }
        _cblock = nullptr;
        _ptr = nullptr;
    }
    void _increment_counter() noexcept {
        if (!_cblock) {
            return;
        }
        ++_cblock->reference_count;
    }
    void _decrement_counter() noexcept {
        if (!_cblock) {
            return;
        }
        if (_cblock->reference_count) {
            --_cblock->reference_count;
        }
    }
};

template <typename T>
class WeakPtr {
public:
    using val_t = T;
    using ref_t = T&;
    using ptr_t = T*;

    constexpr WeakPtr () noexcept
        : _ptr(nullptr), _cblock(nullptr) {}
    WeakPtr (const WeakPtr& other) noexcept
        : _ptr(other._ptr), _cblock(other._cblock) {
        _increment_counter();
    }
    WeakPtr (WeakPtr&& other) noexcept
        : _ptr(other._ptr), _cblock(other._cblock) {
        other._ptr = nullptr;
        other._cblock = nullptr;
    }
    WeakPtr (const SharedPtr<T>& sptr) noexcept {
        _ptr = sptr._ptr;
        _cblock = sptr._cblock;
        _increment_counter();
    }
    WeakPtr& operator= (const WeakPtr& other) noexcept {
        _ptr = other._ptr;
        _cblock = other._cblock;
        _increment_counter();
    }
    WeakPtr& operator= (WeakPtr&& other) noexcept {
        _decrement_counter();
        _clean_up();
        swap(other);
        other._ptr = nullptr;
        other._cblock = nullptr;
    }
    WeakPtr& operator= (const SharedPtr<T>& sptr) noexcept {
        _decrement_counter();
        _clean_up();
        _ptr = sptr._ptr;
        _cblock = sptr._cblock;
        _increment_counter();
        return *this;
    }
    ~WeakPtr () {
        _decrement_counter();
        _clean_up();
    }

    void swap(WeakPtr& other) noexcept {
        std::swap(_ptr, other._ptr);
        std::swap(_cblock, other._cblock);
    }
    void reset() noexcept {
        _decrement_counter();
        _clean_up();
    }

    int use_count () const noexcept {
        return _cblock ? _cblock->reference_count : 0;
    }
    bool expired() const noexcept {
        return !use_count();
    }

    SharedPtr<T> lock () const noexcept {
        if (expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }

    template <typename>
    friend class SharedPtr;
private:
    ptr_t _ptr;
    ControlBlock* _cblock;

    void _clean_up() noexcept {
        if (_cblock && !use_count() && !_cblock->weak_count) {
            delete _cblock;
        }
        _ptr = nullptr;
        _cblock = nullptr;
    }
    void _increment_counter() {
        if (!_cblock) {
            return;
        }
        ++_cblock->weak_count;
    }
    void _decrement_counter() {
        if (!_cblock) {
            return;
        }
        if (_cblock->weak_count) {
            --_cblock->weak_count;
        }
    }
};


