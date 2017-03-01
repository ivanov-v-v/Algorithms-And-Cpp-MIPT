// https://github.com/ivanov-v-v/Deque
// "Writing iterators yourself is hardly ever pretty." (c)

#include <iostream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

template <typename value_t>
class Deque {
private:
    static const int BLOCK_SIZE = 512;
    static const int INIT_BLOCKS_NUMBER = 4;
    static inline int mod_(int i, int modulus) {
        if (i < 0) {
            return (i % modulus + modulus) % modulus;
        }
        return i < modulus ? i : i % modulus;
    }

    class Block {
    public:
        value_t* storage_;
        int back_link_;

        Block() {
            storage_ = new value_t[BLOCK_SIZE];
        }
        Block(const Block& another) {
            storage_ = new value_t[BLOCK_SIZE];
            std::copy(another.storage_, another.storage_ + BLOCK_SIZE, storage_);
        }
        ~Block() {
            delete[] storage_;
            storage_ = nullptr;
        }

        bool operator==(const Block& another) const {
            for (size_t i = 0; i < BLOCK_SIZE; ++i) {
                if (storage_[i] != another.storage_[i]) {
                    return false;
                }
            }
            return true;
        }
        bool operator!=(const Block& another) const {
            return !(*this == another);
        }
    };
    typedef Block* block_ptr;

    int head_, tail_;
    block_ptr *map_;
    int front_block_pos_, back_block_pos_;
    int buffer_capacity_;
    int blocks_stored_;
    int items_stored_;

    inline block_ptr& ptr_at(int i) const {
        return map_[mod_(i, buffer_capacity_)];
    }
    void resize_circular_buffer_(int new_capacity, int blocks_stored) {
        block_ptr *map_copy = new block_ptr[buffer_capacity_]();
        int old_capacity = buffer_capacity_;
        std::copy(map_, map_ + buffer_capacity_, map_copy);

        delete[] map_;
        map_ = new block_ptr[new_capacity]();
        buffer_capacity_ = new_capacity;

        int new_front_block_pos = buffer_capacity_ / 3;
        int new_back_block_pos = new_front_block_pos;

        for (int i = 0, block_id = front_block_pos_; i < blocks_stored; ++i) {
            ptr_at(new_back_block_pos) = map_copy[block_id];
            ptr_at(new_back_block_pos)->back_link_ = new_back_block_pos;
            new_back_block_pos = mod_(new_back_block_pos + 1, buffer_capacity_);
            block_id = mod_(block_id + 1, old_capacity);
        }

        if (new_capacity < old_capacity) {
            int block_id = mod_(back_block_pos_ + 1, old_capacity);
            while (block_id != front_block_pos_) {
                if (map_copy[block_id]) {
                    delete map_copy[block_id];
                    map_copy[block_id] = nullptr;
                }
                block_id = mod_(block_id + 1, old_capacity);
            }
        }

        front_block_pos_ = new_front_block_pos;
        back_block_pos_ = mod_(new_back_block_pos - 1, buffer_capacity_);

        delete[] map_copy;
    }

    template<bool is_const_iterator = true>
    class CustomIterator
            : public std::iterator<std::random_access_iterator_tag,
                    value_t,
                    long long,
                    typename std::conditional<is_const_iterator, const value_t*, value_t*>::type,
                    typename std::conditional<is_const_iterator, const value_t&, value_t&>::type> {
    private:
        const Deque& container_;
        block_ptr bucket_;
        int offset_;

        void assign_offsets(int i) {
            int front_block_pos_ = container_.front_block_pos_;
            int back_block_pos_ = container_.back_block_pos_;

            // code from operator[]
            block_ptr &head = container_.ptr_at(front_block_pos_);
            if (front_block_pos_ == back_block_pos_) {
                bucket_ = head;
                offset_ = (container_.head_ + 1) + i % container_.BLOCK_SIZE;
            } else if (i < container_.BLOCK_SIZE - container_.head_ - 1) {
                bucket_ = head;
                offset_ = container_.head_ + 1 + i;
            } else {
                i -= container_.BLOCK_SIZE - container_.head_ - 1;
                int block_offset = i / container_.BLOCK_SIZE + 1;
                int item_offset = i % container_.BLOCK_SIZE;

                bucket_ = container_.ptr_at(mod_(front_block_pos_ + block_offset, container_.buffer_capacity_));
                offset_ = item_offset;
            }
        }
    public:
        int get_position_() const {
            int block_id = bucket_->back_link_;
            if (block_id == container_.front_block_pos_) {
                return offset_ - container_.head_ - 1;
            }
            if (block_id == container_.back_block_pos_) {
                return container_.items_stored_ - (container_.tail_ - offset_);
            }
            int full_blocks_cnt = 0;
            if (block_id < container_.front_block_pos_) {
                full_blocks_cnt += block_id;
                full_blocks_cnt += container_.buffer_capacity_ - container_.front_block_pos_ - 1;
                return full_blocks_cnt * BLOCK_SIZE + (BLOCK_SIZE - container_.head_) + (offset_ - 1);
            }
            full_blocks_cnt += (block_id - 1) - (container_.front_block_pos_ + 1) + 1;
            return full_blocks_cnt * BLOCK_SIZE + (offset_ - 1) + (BLOCK_SIZE - container_.head_);
        }

        typedef typename std::conditional<is_const_iterator, const value_t&, value_t&>::type reference_type;
        typedef typename std::conditional<is_const_iterator, const value_t*, value_t*>::type pointer_type;

        // getting an Iterator, which points to given position
        CustomIterator(const Deque& some_deque, int i)
            : container_(some_deque) {
            if (i < 0 || i > container_.items_stored_) {
                throw std::logic_error("An attempt to construct an invalid iterator!");
            }
            assign_offsets(i);
        }
        CustomIterator(const CustomIterator<false>& another)
            : container_(another.container_),
              bucket_(another.bucket_),
              offset_(another.offset_)
        {}
        CustomIterator& operator=(const CustomIterator& another) {
            if (this != &another) {
                if (&container_ != &another.container_) {
                    throw std::logic_error("Iterator is attached to the deque it was initialized with!");
                }
                CustomIterator iter_copy(another);
                offset_ = iter_copy.offset_;
                bucket_ = iter_copy.bucket_;
            }
            return *this;
        }

        bool operator==(const CustomIterator& another) const {
            return bucket_ == another.bucket_ &&
                   offset_ == another.offset_ &&
                   &container_ == &another.container_;
        }
        bool operator!=(const CustomIterator& another) const {
            return !(*this == another);
        }

        bool operator<(const CustomIterator& another) const {
            return get_position_() < another.get_position_();
        }
        bool operator>(const CustomIterator& another) const {
            return another < *this;
        }
        bool operator<=(const CustomIterator& another) const {
            return !(*this > another);
        }
        bool operator>=(const CustomIterator& another) const {
            return !(*this < another);
        }

        reference_type operator*() const {
            return bucket_->storage_[offset_];
        }
        pointer_type operator->() const {
            return bucket_->storage_ + offset_;
        }
        reference_type operator[](int n) const {
            return *(*this + n);
        }

        int operator-(const CustomIterator& another) const {
            if (&container_ != &another.container_) {
                throw std::logic_error("Only iterators from the same container are compatible!");
            }
            return get_position_() - another.get_position_();
        }
        CustomIterator& operator++() {
            ++offset_;
            if (offset_ == container_.BLOCK_SIZE) {
                int block_id = bucket_->back_link_;
                block_id = mod_(block_id + 1, container_.buffer_capacity_);
                bucket_ = container_.ptr_at(block_id);
                offset_ = 0;
            }
            return *this;
        }
        CustomIterator operator++(int) {
            CustomIterator old_iterator(*this);
            return ++old_iterator;
        }
        CustomIterator& operator--() {
            --offset_;
            if (offset_ == -1) {
                int block_id = bucket_->back_link_;
                block_id = mod_(block_id - 1, container_.buffer_capacity_);
                bucket_ = container_.ptr_at(block_id);
                offset_ = container_.BLOCK_SIZE - 1;
            }
            return *this;
        }
        CustomIterator operator--(int) {
            CustomIterator old_iterator(*this);
            return --old_iterator;
        }

        CustomIterator& operator+=(int n) {
            assign_offsets(get_position_() + n);
            return *this;
        }
        CustomIterator& operator-=(int n) {
            assign_offsets(get_position_() - n);
            return *this;
        }
        CustomIterator operator+(int n) const {
            return CustomIterator(*this) += n;
        }
        CustomIterator operator-(int n) const  {
            return CustomIterator(*this) -= n;
        }

        friend class CustomIterator<true>;
    };
public:
    typedef CustomIterator<true> const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef CustomIterator<false> iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;

    Deque() {
        map_ = new block_ptr[INIT_BLOCKS_NUMBER]();
        buffer_capacity_ = INIT_BLOCKS_NUMBER;

        front_block_pos_ = INIT_BLOCKS_NUMBER / 2;
        back_block_pos_ = front_block_pos_;

        ptr_at(front_block_pos_) = new Block();
        ptr_at(front_block_pos_)->back_link_ = front_block_pos_;
        head_ = -1, tail_ = 0;

        blocks_stored_ = 1;
        items_stored_ = 0;
    }
    Deque(const Deque& another) {
        map_ = new block_ptr[another.buffer_capacity_]();

        front_block_pos_ = another.front_block_pos_;
        back_block_pos_ = another.back_block_pos_;

        head_ = another.head_, tail_ = another.tail_;
        items_stored_ = another.items_stored_;
        buffer_capacity_ = another.buffer_capacity_;
        blocks_stored_ = another.blocks_stored_;

        ptr_at(front_block_pos_) = new Block(*another.ptr_at(front_block_pos_));
        ptr_at(front_block_pos_)->back_link_ = front_block_pos_;

        if (front_block_pos_ != back_block_pos_) {
            for (int i = mod_(front_block_pos_ + 1, another.buffer_capacity_);
                 i != back_block_pos_; i = mod_(i + 1, another.buffer_capacity_)) {
                ptr_at(i) = new Block(*another.map_[i]);
                ptr_at(i)->back_link_ = i;
            }
            ptr_at(back_block_pos_) = new Block(*another.map_[back_block_pos_]);
            ptr_at(back_block_pos_)->back_link_ = back_block_pos_;
        }
    }
    Deque& operator=(const Deque& another) {
        if (this != &another) {
            Deque<value_t> copied(another);
            buffer_capacity_ = copied.buffer_capacity_;
            front_block_pos_ = copied.front_block_pos_;
            back_block_pos_ = copied.back_block_pos_;
            blocks_stored_ = copied.blocks_stored_;
            items_stored_ = copied.items_stored_;
            head_ = copied.head_, tail_ = copied.tail_;

            delete[] map_;
            map_ = new Block*[copied.buffer_capacity_]();
            for (size_t i = 0; i < copied.buffer_capacity_; ++i) {
                if (copied.map_[i]) {
                    map_[i] = new Block(*copied.map_[i]);
                    map_[i]->back_link_ = i;
                }
            }
        }
        return *this;
    }
    ~Deque() {
        for (int i = 0; i < buffer_capacity_; ++i) {
            if (map_[i]) {
                delete map_[i];
                map_[i] = nullptr;
            }
        }
        delete[] map_;
    }

    bool empty() const {
        return !items_stored_;
    }
    int size() const {
        return items_stored_;
    }

    iterator begin() {
        return iterator(*this, 0);
    }
    const iterator begin() const {
        return iterator(*this, 0);
    }
    iterator end() {
        return iterator(*this, items_stored_);
    }
    const iterator end() const {
        return iterator(*this, items_stored_);
    }
    reverse_iterator rbegin() {
        return std::reverse_iterator<iterator>(end());
    }
    const reverse_iterator rbegin() const {
        return std::reverse_iterator<iterator>(end());
    }
    reverse_iterator rend() {
        return std::reverse_iterator<iterator>(begin());
    }
    const reverse_iterator rend() const {
        return std::reverse_iterator<iterator>(begin());
    }

    const_iterator cbegin() {
        return const_iterator(*this, 0);
    }
    const const_iterator cbegin() const {
        return const_iterator(*this, 0);
    }
    const_iterator cend() {
        return const_iterator(*this, items_stored_);
    }
    const const_iterator cend() const {
        return const_iterator(*this, items_stored_);
    }
    const_reverse_iterator crbegin() {
        return std::reverse_iterator<const_iterator>(cend());
    }
    const const_reverse_iterator crbegin() const {
        return std::reverse_iterator<const_iterator>(cend());
    }
    const_reverse_iterator crend() {
        return std::reverse_iterator<const_iterator>(cbegin());
    }
    const const_reverse_iterator crend() const {
        return std::reverse_iterator<const_iterator>(cbegin());
    }

    bool operator==(const Deque& another) const {
        if (items_stored_ != another.items_stored_) {
            return false;
        }
        return std::equal(begin(), end(), another.begin());
    }
    bool operator!=(const Deque& another) {
        return !(*this == another);
    }

    void push_back(const value_t& item) {
        if (tail_ == BLOCK_SIZE) {
            tail_ = 0;
            if (blocks_stored_ + 1 > buffer_capacity_) {
                resize_circular_buffer_(buffer_capacity_ * 2, blocks_stored_);
            }
            ++blocks_stored_;
            back_block_pos_ = mod_(back_block_pos_ + 1, buffer_capacity_);
            ptr_at(back_block_pos_) = new Block();
            ptr_at(back_block_pos_)->back_link_ = back_block_pos_;
        }
        block_ptr& tail = ptr_at(back_block_pos_);
        tail->storage_[tail_++] = item;
        ++items_stored_;
    }
    void pop_back() {
        if (empty()) {
            throw std::logic_error("Can't pop from the empty deque!");
        }
        if (!tail_) {
            tail_ = BLOCK_SIZE;
            if (4 * (blocks_stored_ - 1) == buffer_capacity_) {
                resize_circular_buffer_(buffer_capacity_ / 2, blocks_stored_);
            }
            --blocks_stored_;
            back_block_pos_ = mod_(back_block_pos_ - 1, buffer_capacity_);
        }
        tail_ = mod_(tail_ - 1, BLOCK_SIZE);
        --items_stored_;
    }
    void push_front(const value_t& item) {
        if (head_ == -1) {
            head_ = BLOCK_SIZE - 1;
            if (blocks_stored_ + 1 > buffer_capacity_) {
                resize_circular_buffer_(buffer_capacity_ * 2, blocks_stored_);
            }
            ++blocks_stored_;
            front_block_pos_ = mod_(front_block_pos_ - 1, buffer_capacity_);
            ptr_at(front_block_pos_) = new Block();
            ptr_at(front_block_pos_)->back_link_ = front_block_pos_;
        }
        block_ptr& head = ptr_at(front_block_pos_);
        head->storage_[head_--] = item;
        ++items_stored_;
    }
    void pop_front() {
        if (empty()) {
            throw std::logic_error("Can't pop from the empty deque!");
        }
        if (head_ == BLOCK_SIZE - 1) {
            if (4 * (blocks_stored_ - 1) == buffer_capacity_) {
                resize_circular_buffer_(buffer_capacity_ / 2, blocks_stored_);
            }
            --blocks_stored_;
            front_block_pos_ = mod_(front_block_pos_ + 1, buffer_capacity_);
        }
        head_ = mod_(head_ + 1, BLOCK_SIZE);
        --items_stored_;
    }

    value_t& operator[](int i) {
        if (i < 0 || i >= items_stored_) {
            throw std::logic_error("Incorrect position in operator[]");
        }
        return begin()[i];
    }
    const value_t& operator[](int i) const {
        if (i < 0 || i >= items_stored_) {
            throw std::logic_error("Incorrect position in operator[]");
        }
        return cbegin()[i];
    }

    value_t& back() {
        return *rbegin();
    }
    const value_t& back() const {
        return *crbegin();
    }
    value_t& front() {
        return *begin();
    }
    const value_t& front() const {
        return *cbegin();
    }
    void display_storage() const {
        for (auto it = begin(); it != end(); ++it) {
            std::cout << *it;
            if (it + 1 != end()) {
                std::cout << " ";
            }
        } std::cout << std::endl;
    }
};

template <typename value_t>
typename Deque<value_t>::const_iterator operator+(int n, const typename Deque<value_t>::const_iterator& it) {
    return it + n;
}

template <typename value_t>
typename Deque<value_t>::iterator operator+(int n, const typename Deque<value_t>::iterator& it) {
    return it + n;
}

