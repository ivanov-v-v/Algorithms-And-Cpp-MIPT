#include <iostream>
#include <vector>
#include <string>

template<typename T>
inline void swap_(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}
template<typename T>
inline T max_(const T& a, const T& b) {
    return b < a ? a : b;
}
template<typename T>
inline T min_(const T& a, const T& b) {
    return a < b ? a : b;
}

class BigInteger {
private:
    static const int BASE = 10;
    enum signT {minus, plus};

    std::vector<int> a;
    signT sign;

    void removeLeadingZeros_() {
        while (!a.empty() && !a.back()) {
            a.pop_back();
        }
        if (a.empty()) {
            sign = plus;
        }
    }
    BigInteger& unsignedPlus_(const BigInteger& x) {
        int n = a.size(), m = x.a.size();
        int carry = 0;
        for (int i = 0; i < max_(n, m) || carry; ++i) {
            int toAdd = carry;
            if (i < n) {
                toAdd += a[i];
            }
            if (i < m) {
                toAdd += x.a[i];
            }
            if (i < n) {
                a[i] = toAdd % BASE;
            } else {
                a.push_back(toAdd % BASE);
            }
            carry = toAdd / BASE;
        }
        return *this;
    }
    // assumed: lhs > rhs
    BigInteger& unsignedSubtract_(const BigInteger& x) {
        int n = a.size(), m = x.a.size();
        int carry = 0;
        for (int i = 0; i < max_(n, m) || carry; ++i) {
            int toAdd = a[i] - carry;
            if (i < m) {
                toAdd -= x.a[i];
            }
            if (toAdd < 0) {
                carry = 1;
                toAdd += 10;
            } else {
                carry = 0;
            }
            if (i < n) {
                a[i] = toAdd;
            } else {
                a.push_back(toAdd);
            }
        }
        removeLeadingZeros_();
        return *this;
    }
    // assumed: k <= 1e8
    BigInteger& multiplyByInt_(int k) {
        if (!k || !*this) {
            a.clear();
            sign = plus;
            return *this;
        }
        if (k < 0) {
            k *= -1;
            sign = (sign == plus ? minus : plus);
        }
        int n = a.size();
        int carry = 0;
        for (int i = 0; i < n || carry; ++i) {
            int toAdd = carry;
            if (i < n) {
                toAdd += a[i] * k;
            } if (i < n) {
                a[i] = toAdd % BASE;
            } else {
                a.push_back(toAdd % BASE);
            }
            carry = toAdd / BASE;
        }
        return *this;
    }
public:
    BigInteger() {
        sign = plus;
    }
    BigInteger(int x) {
        sign = (x >= 0) ? plus : minus;
        if (x < 0) {
            x *= -1;
        }
        while (x) {
            a.push_back(x % 10);
            x /= 10;
        }
    }
    BigInteger(const std::string& s) {
        sign = plus;
        if (s != "0") {
            int n = s.size();
            for (int i = n - 1; i >= 0; --i) {
                a.push_back(s[i] - '0');
            }
            if (!s.empty() && s[0] == '-') {
                a.pop_back();
                sign = minus;
            }
        }
    }
    BigInteger(const BigInteger& x) {
        sign = x.sign;
        a = x.a;
    }
    BigInteger& operator=(const BigInteger& x) {
        if (this != &x) {
            sign = x.sign;
            a = x.a;
        }
        return *this;
    }

    explicit operator bool() const {
        return !a.empty();
    }

    std::string toString() const {
        if (a.empty()) {
            return "0";
        }
        std::string s = negative() ? "-" : "";
        int n = a.size();
        for (int i = n - 1; i >= 0; --i) {
            s += static_cast<char>(a[i] + '0');
        }
        return s;
    }
    friend std::istream& operator>>(std::istream& in, BigInteger& x);

    inline bool positive() const {
        return sign == plus && !a.empty();
    }
    inline bool isZero() const {
        return a.empty();
    }
    inline bool negative() const {
        return sign == minus;
    }
    const BigInteger absVal() const {
        BigInteger absValue;
        absValue.sign = plus;
        absValue.a = a;
        return absValue;
    }
    unsigned length() const {
        return a.size();
    }
    int getDigit(int i) const {
        return a[i];
    }

    const BigInteger operator-() const {
        BigInteger neg(*this);
        if (neg) {
            neg.sign = (neg.sign == plus ? minus : plus);
        }
        return neg;
    }

    bool operator ==(const BigInteger& rhs) const {
        return sign == rhs.sign && a == rhs.a;
    }
    bool operator !=(const BigInteger& rhs) const {
        return !(*this == rhs);
    }
    bool operator <(const BigInteger& rhs) const {
        if (sign != rhs.sign) {
            return sign == minus;
        }
        if (a.size() != rhs.a.size()) {
            return (a.size() < rhs.a.size()) ^ (sign == minus);
        }
        int n = a.size();
        for (int i = n - 1; i >= 0; --i) {
            if (a[i] != rhs.a[i]) {
                return (a[i] < rhs.a[i]) ^ (sign == minus);
            }
        }
        return false;
    }
    bool operator <=(const BigInteger& rhs) const {
        return !(*this > rhs);
    }
    bool operator >(const BigInteger& rhs) const {
        return rhs < *this;
    }
    bool operator >=(const BigInteger& rhs) const {
        return !(*this < rhs);
    }

    BigInteger& operator+=(const BigInteger& rhs) {
        if (!*this || !rhs) {
            return *this ? *this : *this = rhs;
        }
        if (sign == rhs.sign) {
            return this->unsignedPlus_(rhs);
        }
        if (absVal() == rhs.absVal()) {
            a.clear();
            sign = plus;
            return *this;
        } else if (negative()) {
            if (absVal() < rhs.absVal()) {
                BigInteger rhsCopy(rhs);
                swap(this->a, rhsCopy.a);
                this->unsignedSubtract_(rhsCopy);
                sign = plus;
            } else {
                unsignedSubtract_(rhs);
                sign = minus;
            }
        } else {
            if (absVal() < rhs.absVal()) {
                BigInteger rhsCopy(rhs);
                swap(this->a, rhsCopy.a);
                this->unsignedSubtract_(rhsCopy);
                sign = minus;
            } else {
                this->unsignedSubtract_(rhs);
                sign = plus;
            }
        }
        return *this;
    }
    BigInteger& operator-=(const BigInteger& rhs) {
        return *this += -rhs;
    }
    BigInteger& operator*=(const BigInteger& rhs) {
        if (!*this || !rhs) {
            a.clear();
            sign = plus;
            return *this;
        }
        if (rhs.a.size() < 8) {
            int k = 0;
            for (int i = rhs.a.size() - 1; i >= 0; --i) {
                k = k * 10 + rhs.a[i];
            }
            return this->multiplyByInt_(k * (rhs.sign == minus ? -1 : 1));
        }
        unsigned n = a.size(), m = rhs.a.size();
        BigInteger toAdd(n < m ? rhs : *this), prod;
        toAdd.sign = plus;
        signT prodSign = (sign == rhs.sign ? plus : minus);
        std::vector<int> lhs = a;
        a.clear();
        sign = plus;
        for (unsigned i = 0; i < min_(n, m); i++) {
            *this += BigInteger(toAdd).multiplyByInt_(n < m ? lhs[i] : rhs.a[i]);
            toAdd.multiplyByInt_(10);
        }
        sign = *this ? prodSign : plus;
        return *this;
    }
    BigInteger& operator/=(const BigInteger& rhs) {
        if (!*this) {
            return *this;
        }
        BigInteger z, absRhs = rhs.absVal();
        signT div_sign = sign == rhs.sign ? plus : minus;
        int n = a.size();
        std::vector<int> lhs = a;
        a.clear();
        sign = plus;
        bool divides = false;
        for (int i = 0; i < n; i++) {
            z.multiplyByInt_(10);
            z += lhs[n - i - 1];
            if (divides || z >= absRhs) {
                divides = true;
                this->multiplyByInt_(10);
                while (z >= absRhs) {
                    z -= absRhs;
                    ++(*this);
                }
            }
        }
        sign = *this ? div_sign : plus;
        return *this;
    }
    BigInteger& operator%=(const BigInteger& rhs) {
        if (!*this) {
            return *this;
        }
        BigInteger quotient(*this);
        quotient /= rhs;
        BigInteger multipleToModulus(rhs);
        multipleToModulus *= quotient;
        return *this -= multipleToModulus;
    }

    BigInteger& operator++() {
        return *this += 1;
    }
    const BigInteger operator++(int) {
        BigInteger old(*this);
        ++(*this);
        return old;
    }
    BigInteger& operator--() {
        return *this -= 1;
    }
    const BigInteger operator--(int) {
        BigInteger old(*this);
        --(*this);
        return old;
    }
};

// I/O
std::istream& operator>>(std::istream& in, BigInteger& x) {
    std::string s;
    in >> s;
    x = s;
    return in;
}
std::ostream& operator<<(std::ostream& out, const BigInteger& x) {
    out << x.toString();
    return out;
}

const BigInteger operator+(const BigInteger& lhs, const BigInteger& rhs) {
    return BigInteger(lhs) += rhs;
}
const BigInteger operator-(const BigInteger& lhs, const BigInteger& rhs) {
    return BigInteger(lhs) -= rhs;
}
const BigInteger operator*(const BigInteger& lhs, const BigInteger& rhs) {
    return BigInteger(lhs) *= rhs;
}
const BigInteger operator/(const BigInteger& lhs, const BigInteger& rhs) {
    return BigInteger(lhs) /= rhs;
}
const BigInteger operator%(const BigInteger& lhs, const BigInteger& rhs) {
    return BigInteger(lhs) %= rhs;
}

const BigInteger gcd(const BigInteger& a, const BigInteger& b) {
    BigInteger a_ = a.absVal(), b_ = b.absVal();
    while (b_) {
        a_ %= b_;
        swap_(a_, b_);
    }
    return a_;
}
inline const BigInteger lcm(const BigInteger& a, const BigInteger& b) {
    if (!a && !b) return BigInteger(0);
    return (a * (b / gcd(a, b)));
}

class Permutation {
private:
    // 0      1    ...    n - 1
    // p[0]   p[1]        p[n - 1]
    unsigned n_;
    int *p_;

    // subroutine for inversions counting
    size_t mergeCount_(int* start, int* finish, int* buffer) const {
        unsigned intervalLength = finish - start;
        if (intervalLength <= 1) {
            return 0;
        }
        int* middle = start + intervalLength / 2;
        size_t inversions = 0;
        inversions += mergeCount_(start, middle, buffer);
        inversions += mergeCount_(middle, finish, buffer);
        int *lp = start, *rp = middle;
        for (size_t i = 0; i < intervalLength; ++i) {
            if (rp == finish) {
                buffer[i] = *lp++;
                inversions += rp - middle;
            } else if (lp == middle) {
                buffer[i] = *rp++;
            } else {
                if (*lp <= *rp) {
                    buffer[i] = *lp++;
                    inversions += rp - middle;
                } else {
                    buffer[i] = *rp++;
                }
            }
        }
        for (size_t i = 0; start + i != finish; ++i) {
            *(start + i) = buffer[i];
        }
        return inversions;
    }
    enum orderChangeType { toDescending, toAscending };
    Permutation reorderSuffix(orderChangeType manner) const {
        // l: last position such that p_[i]
        // violates the ordering of suffix elements
        int l = -1;
        for (int i = n_ - 2; i >= 0; --i) {
            if ((p_[i] > p_[i + 1]) ^ (manner == toAscending)) {
                l = i;
                break;
            }
        }
        // r: last position such that p[l] is greater (smaller) than p[r]
        int r = -1;
        for (int i = n_ - 1; i > l; --i) {
            if ((p_[l] > p_[i]) ^ (manner == toAscending)) {
                r = i;
                break;
            }
        }
        // swap l-th with r-th and reverse the suffix
        Permutation updated(n_, p_);
        swap_(updated.p_[l], updated.p_[r]);
        ++l;
        r = n_ - 1;
        while (l < r) {
            swap_(updated.p_[l], updated.p_[r]);
            ++l;
            --r;
        }
        return updated;
    }
public:
    Permutation(): n_(0), p_(nullptr) {}
    explicit Permutation(unsigned length) {
        n_ = length;
        p_ = new int[n_];
        for (size_t i = 0; i < n_; ++i) {
            p_[i] = i;
        }
    }
    Permutation(unsigned length, int* a) {
        n_ = length;
        p_ = new int[n_];
        for (size_t i = 0; i < n_; ++i) {
            p_[i] = a[i];
        }
    }
    Permutation(unsigned m, BigInteger lexNumber) {
        n_ = m;
        p_ = new int[n_];
        BigInteger factorial = 1;
        for (size_t i = 1; i <= n_; ++i) {
            factorial *= i;
        }
        bool *used = new bool[n_];
        for (size_t i = 0; i < n_; ++i) {
            used[i] = 0;
        }
        for (size_t i = 0; i < n_; ++i) {
            factorial /= (n_ - i);
            int k = 0;
            while (lexNumber >= factorial) {
                lexNumber -= factorial;
                ++k;
            }
            int digit = 0;
            for (; k || used[digit]; ++digit)  {
                if (!used[digit]) --k;
            }
            p_[i] = digit;
            used[digit] = 1;
        }
        delete[] used;
    }
    Permutation(const Permutation& other) {
        n_ = other.n_;
        p_ = new int[n_];
        for (size_t i = 0; i < n_; ++i) {
            p_[i] = other.p_[i];
        }
    }
    Permutation& operator=(const Permutation& tau) {
        if (this != &tau) {
            Permutation x(tau.n_, tau.p_);
            swap_(n_, x.n_);
            swap_(p_, x.p_);
        }
        return *this;
    }
    ~Permutation() {
        delete[] p_;
    }

    inline unsigned length() const {
        return n_;
    }
    inline int at(int i) const {
        return p_[i];
    }

    template<typename T>
    void operator()(T* data) const {
        T* rearranged = new T[n_];
        for (size_t i = 0; i < n_; ++i) {
            rearranged[p_[i]] = data[i];
        }
        for (size_t i = 0; i < n_; ++i) {
            data[i] = rearranged[i];
        }
        delete[] rearranged;
    }
    int operator[](size_t i) const {
        return p_[i];
    }
    BigInteger getLexNumber() const {
        BigInteger lexNumber = 0;
        BigInteger factorial = 1;
        for (size_t i = 2; i <= n_; ++i) {
            factorial *= i;
        }
        bool* used = new bool[n_];
        for (size_t i = 0; i < n_; ++i) {
            factorial /= (n_ - i);
            int ord = 0;
            for (int j = 0; j < p_[i]; j++) {
                if (!used[j]) {
                    ++ord;
                }
            }
            lexNumber += ord * factorial;
            used[p_[i]] = 1;
        }
        delete[] used;
        return lexNumber;
    }
    const Permutation inverse() const {
        Permutation inversePermutation(n_);
        for (size_t i = 0; i < n_; ++i) {
            inversePermutation.p_[p_[i]] = i;
        }
        return inversePermutation;
    }
    bool operator==(const Permutation& tau) const {
        for (size_t i = 0; i < n_; ++i) {
            if (p_[i] != tau[i]) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const Permutation& tau) const {
        return !(*this == tau);
    }
    // lexicographical comparison
    bool operator<(const Permutation& tau) const {
        for (size_t i = 0; i < n_; ++i) {
            if (p_[i] != tau[i]) {
                return p_[i] < tau[i];
            }
        }
        return false;
    }
    bool operator>(const Permutation& tau) const {
        return tau < *this;
    }
    bool operator<=(const Permutation& tau) const {
        return !(tau < *this);
    }
    bool operator>=(const Permutation& tau) const {
        return !(*this < tau);
    }
    // composition of permutations
    Permutation& operator *=(const Permutation& tau) {
        int *composition = new int[n_];
        for (size_t i = 0; i < n_; ++i) {
            composition[i] = p_[tau[i]];
        }
        for (size_t i = 0; i < n_; ++i) {
            p_[i] = composition[i];
        }
        delete[] composition;
        return *this;
    }
    const Permutation operator*(const Permutation& tau) const {
        Permutation composition(*this);
        return composition *= tau;
    }
    Permutation& operator+=(int count) {
        return *this = Permutation(n_, BigInteger(getLexNumber() + count));
    }
    Permutation& operator-=(int count) {
        return *this = Permutation(n_, BigInteger(getLexNumber() + count));
    }
    const Permutation operator+(int count) const {
        return Permutation(*this) += count;
    }
    const Permutation operator-(int count) const {
        return Permutation(*this) -= count;
    }

    const Permutation next() const {
        return reorderSuffix(toAscending);
    }
    const Permutation previous() const {
        return reorderSuffix(toDescending);
    }
    Permutation& operator++() {
        return *this = this->next();
    }
    const Permutation operator++(int) {
        Permutation old(*this);
        *this = this->next();
        return old;
    }
    Permutation& operator--() {
        return *this = this->previous();
    }
    const Permutation operator--(int) {
        Permutation old(*this);
        *this = this->previous();
        return old;
    }

    size_t inversionsCount() const {
        int* p_copy = new int[n_];
        for (size_t i = 0; i < n_; ++i) {
            p_copy[i] = p_[i];
        }
        int* buffer = new int[n_];
        size_t answer = mergeCount_(p_copy, p_copy + n_, buffer);
        delete[] buffer;
        delete[] p_copy;
        return answer;
    }
    bool isOdd() const {
        int parity = 0;
        bool* used = new bool[n_];
        for (size_t i = 0; i < n_; ++i) {
            used[i] = 0;
        }
        for (size_t i = 0; i < n_; ++i) {
            if (used[i]) {
                continue;
            }
            size_t cycleLength = 0, v = i;
            while (!used[v]) {
                used[v] = 1;
                ++cycleLength;
                v = p_[v];
            }
            parity ^= (cycleLength % 2 == 0);
        }
        delete[] used;
        return parity == 1;
    }
    bool isEven() const {
        return !isOdd();
    }
    Permutation pow(int degree) const {
        Permutation sigma (degree < 0 ? inverse() : *this);
        if (degree < 0) {
            degree *= -1;
        }
        Permutation tau(n_);
        bool* used = new bool[n_];
        int* cycle = new int[n_];
        for (size_t i = 0; i < n_; ++i) {
            used[i] = 0;
        }
        for (size_t i = 0; i < n_; ++i) {
            if (used[i]) {
                continue;
            }
            size_t id = i;
            int cycle_len = 0;
            while (!used[id]) {
                used[id] = true;
                id = sigma[id];
                cycle[cycle_len++] = id;
            }
            int shift = degree % cycle_len;
            if (shift) {
                for (int i = 0; i < cycle_len; ++i) {
                    tau.p_[cycle[i]] = sigma[cycle[(i + shift - 1) % cycle_len]];
                }
            }
        }
        delete[] cycle;
        delete[] used;
        return tau;
    }
};

std::ostream& operator<<(std::ostream& out, const Permutation& sigma) {
    for (size_t i = 0; i < sigma.length(); ++i) {
        out << sigma.at(i);
        if (i + 1 != sigma.length()) {
            out << ' ';
        }
    }
    return out;
}
