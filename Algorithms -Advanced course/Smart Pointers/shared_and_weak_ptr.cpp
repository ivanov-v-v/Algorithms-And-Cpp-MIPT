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

