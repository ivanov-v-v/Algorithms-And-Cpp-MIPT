/* Arbitrary length numbers. Fractions.
 * BigInteger:
 * - based on std::vector container;
 * - no digit compression;
 * - multiplication, divison: O(len ^ 2).
 * Rational:
 * - supports representation with arbitrary precision;
 * - supports explicit casting to double.
 * */

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

class BigInteger {
private:
    static const int BASE = 10;
    enum signT {minus, plus};

    std::vector<int> digits_;
    signT sign_;
//--------------------------------------------------------------------------
// Subroutines of arithmetical operations
    void removeLeadingZeros_() {
        while (!digits_.empty() && !digits_.back()) {
            digits_.pop_back();
        }
        if (digits_.empty()) {
            sign_ = plus;
        }
    }
    // add numbers and save the result to lhs
    void unsignedPlus_(const BigInteger& x) {
        int n = digits_.size(), m = x.digits_.size();
        int carry = 0;
        for (int i = 0; i < std::max(n, m) || carry; ++i) {
            int toAdd = carry;
            if (i < n) {
                toAdd += digits_[i];
            }
            if (i < m) {
                toAdd += x.digits_[i];
            }
            if (i < n) {
                digits_[i] = toAdd % BASE;
            } else {
                digits_.push_back(toAdd % BASE);
            }
            carry = toAdd / BASE;
        }
    }
    // assumed: lhs > rhs; subtract and save the result to lhs
    void unsignedSubtract_(const BigInteger& x) {
        int n = digits_.size(), m = x.digits_.size();
        int carry = 0;
        for (int i = 0; i < std::max(n, m) || carry; ++i) {
            int toAdd = digits_[i] - carry;
            if (i < m) {
                toAdd -= x.digits_[i];
            }
            if (toAdd < 0) {
                carry = 1;
                toAdd += 10;
            } else {
                carry = 0;
            }
            if (i < n) {
                digits_[i] = toAdd;
            } else {
                digits_.push_back(toAdd);
            }
        }
        removeLeadingZeros_();
    }
    // assumed: k <= 1e8
    void multiplyByInt_(int k) {
        if (!k || !*this) {
            digits_.clear();
            sign_ = plus;
            return;
        }
        if (k < 0) {
            k *= -1;
            sign_ = (sign_ == plus ? minus : plus);
        }
        int n = digits_.size();
        int carry = 0;
        for (int i = 0; i < n || carry; ++i) {
            int toAdd = carry;
            if (i < n) {
                toAdd += digits_[i] * k;
            } if (i < n) {
                digits_[i] = toAdd % BASE;
            } else {
                digits_.push_back(toAdd % BASE);
            }
            carry = toAdd / BASE;
        }
    }
//--------------------------------------------------------------------------
public:
// Constructors
    BigInteger() {
        sign_ = plus;
    }
    BigInteger(int x) {
        sign_ = (x >= 0) ? plus : minus;
        if (x < 0) {
            x *= -1;
        }
        while (x) {
            digits_.push_back(x % 10);
            x /= 10;
        }
    }
    BigInteger(const std::string& s) {
        sign_ = plus;
        if (s != "0") {
            int n = s.size();
            for (int i = n - 1; i >= 0; --i) {
                digits_.push_back(s[i] - '0');
            }
            if (!s.empty() && s[0] == '-') {
                digits_.pop_back();
                sign_ = minus;
            }
        }
    }
    BigInteger(const BigInteger& x) {
        sign_ = x.sign_;
        digits_ = x.digits_;
    }
//--------------------------------------------------------------------------
// Assignment operators
    BigInteger& operator=(const BigInteger& x) {
        if (this != &x) {
            sign_ = x.sign_;
            digits_ = x.digits_;
        }
        return *this;
    }
//--------------------------------------------------------------------------
// Type casting
    explicit operator bool() const {
        return !digits_.empty();
    }
//--------------------------------------------------------------------------
// I/O
    std::string toString() const {
        if (digits_.empty()) {
            return "0";
        }
        std::string s = negative() ? "-" : "";
        int n = digits_.size();
        for (int i = n - 1; i >= 0; --i) {
            s += static_cast<char>(digits_[i] + '0');
        }
        return s;
    }
    friend std::istream& operator>>(std::istream& in, BigInteger& x);
//--------------------------------------------------------------------------
// Characteristics
    inline bool positive() const {
        return sign_ == plus && !digits_.empty();
    }
    inline bool isZero() const {
        return digits_.empty();
    }
    inline bool negative() const {
        return sign_ == minus;
    }
    const BigInteger absVal() const {
        BigInteger absValue;
        absValue.sign_ = plus;
        absValue.digits_ = digits_;
        return absValue;
    }
    unsigned length() const {
        return digits_.size();
    }
    int getDigit(int i) const {
        return digits_[i];
    }
//--------------------------------------------------------------------------
// Logical operators
    bool operator ==(const BigInteger& rhs) const {
        return sign_ == rhs.sign_ && digits_ == rhs.digits_;
    }
    bool operator !=(const BigInteger& rhs) const {
        return !(*this == rhs);
    }
    bool operator <(const BigInteger& rhs) const {
        if (sign_ != rhs.sign_) {
            return sign_ == minus;
        }
        if (digits_.size() != rhs.digits_.size()) {
            return (digits_.size() < rhs.digits_.size()) ^ (sign_ == minus);
        }
        int n = digits_.size();
        for (int i = n - 1; i >= 0; --i) {
            if (digits_[i] != rhs.digits_[i]) {
                return (digits_[i] < rhs.digits_[i]) ^ (sign_ == minus);
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
//--------------------------------------------------------------------------
// Arithmetical operators
    const BigInteger operator-() const {
        BigInteger neg(*this);
        if (neg) {
            neg.sign_ = (neg.sign_ == plus ? minus : plus);
        }
        return neg;
    }
// Modifying
    BigInteger& operator+=(const BigInteger& rhs) {
        if (!*this || !rhs) {
            return *this ? *this : *this = rhs;
        }
        if (sign_ == rhs.sign_) {
            unsignedPlus_(rhs);
            return *this;
        }
        if (absVal() == rhs.absVal()) {
            digits_.clear();
            sign_ = plus;
            return *this;
        } else if (negative()) {
            if (absVal() < rhs.absVal()) {
                BigInteger rhsCopy(rhs);
                std::swap(digits_, rhsCopy.digits_);
                unsignedSubtract_(rhsCopy);
                sign_ = plus;
            } else {
                unsignedSubtract_(rhs);
                sign_ = minus;
            }
        } else {
            if (absVal() < rhs.absVal()) {
                BigInteger rhsCopy(rhs);
                std::swap(digits_, rhsCopy.digits_);
                unsignedSubtract_(rhsCopy);
                sign_ = minus;
            } else {
                unsignedSubtract_(rhs);
                sign_ = plus;
            }
        }
        return *this;
    }
    BigInteger& operator-=(const BigInteger& rhs) {
        return *this += -rhs;
    }
    BigInteger& operator*=(const BigInteger& rhs) {
        if (!*this || !rhs) {
            digits_.clear();
            sign_ = plus;
            return *this;
        }
        if (rhs.digits_.size() < 8) {
            int k = 0;
            for (int i = rhs.digits_.size() - 1; i >= 0; --i) {
                k = k * 10 + rhs.digits_[i];
            }
            multiplyByInt_(k * (rhs.sign_ == minus ? -1 : 1));
            return *this;
        }
        unsigned n = digits_.size(), m = rhs.digits_.size();
        BigInteger toAdd(n < m ? rhs : *this), prod;
        toAdd.sign_ = plus;
        signT prodSign = (sign_ == rhs.sign_ ? plus : minus);
        std::vector<int> lhs = digits_;
        digits_.clear();
        sign_ = plus;
        for (unsigned i = 0; i < std::min(n, m); i++) {
            BigInteger addend(toAdd);
            addend.multiplyByInt_(n < m ? lhs[i] : rhs.digits_[i]);
            *this += addend;
            toAdd.multiplyByInt_(10);
        }
        sign_ = *this ? prodSign : plus;
        return *this;
    }
    BigInteger& operator/=(const BigInteger& rhs) {
        if (!*this) {
            return *this;
        }
        BigInteger z, absRhs = rhs.absVal();
        signT div_sign = sign_ == rhs.sign_ ? plus : minus;
        int n = digits_.size();
        std::vector<int> lhs = digits_;
        digits_.clear();
        sign_ = plus;
        bool divides = false;
        for (int i = 0; i < n; i++) {
            z.multiplyByInt_(10);
            z += lhs[n - i - 1];
            if (divides || z >= absRhs) {
                divides = true;
                multiplyByInt_(10);
                while (z >= absRhs) {
                    z -= absRhs;
                    ++(*this);
                }
            }
        }
        sign_ = *this ? div_sign : plus;
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
//--------------------------------------------------------------------------
// Arithmetical operators (non-modifying)
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
        std::swap(a_, b_);
    }
    return a_;
}
inline const BigInteger lcm(const BigInteger& a, const BigInteger& b) {
    if (!a && !b) return BigInteger(0);
    return (a * (b / gcd(a, b)));
}
//--------------------------------------------------------------------------

class Rational {
private:
    BigInteger nom, denom;
    void reduce_() {
        if (denom.negative()) {
            if (nom) {
                nom *= -1;
            }
            denom = denom.absVal();
        }
        BigInteger g = gcd(nom, denom);
        nom /= g;
        denom /= g;
    }
//--------------------------------------------------------------------------
public:
// Constructors
    Rational() {
        nom = BigInteger(0);
        denom = BigInteger(1);
    }
    Rational(const BigInteger& a, const BigInteger& b) {
        if (!a) {
            nom = 0, denom = 1;
        } else {
            nom = (a.negative() ^ b.negative()) ? -a.absVal() : a.absVal();
            denom = b.absVal();
            reduce_();
        }
    }
    Rational(const BigInteger& x) {
        nom = BigInteger(x);
        denom = BigInteger(1);
    }
    Rational(int x) {
        nom = BigInteger(x);
        denom = BigInteger(1);
    }
    Rational(const Rational& other) {
        nom = other.nom;
        denom = other.denom;
    }
    Rational& operator=(const Rational& x) {
        if (this != &x) {
            nom = x.nom;
            denom = x.denom;
        }
        return *this;
    }
//--------------------------------------------------------------------------
// Type casting
    explicit operator double() const {
        double res = 0;
        BigInteger lhs(nom.absVal()), rhs(denom.absVal()), z;
        bool divides = false;
        unsigned n = lhs.length();
        for (size_t i = 0; i < n; ++i) {
            z = z * 10 + lhs.getDigit(n - i - 1);
            if (divides || z >= rhs) {
                divides = true;
                int k = 0;
                while (z >= rhs) {
                    z -= rhs;
                    ++k;
                }
                res = res * 10. + k;
            }
        }
        double shift = 0.1;
        for (size_t i = 0; z && i < 50; ++i) {
            z *= 10;
            if (z >= rhs) {
                int k = 0;
                while (z >= rhs) {
                    z -= rhs;
                    k++;
                }
                res += k * shift;
            }
            shift /= 10.;
        }
        return nom.negative() ? -res : res;
    }
//--------------------------------------------------------------------------
    std::string asDecimal(size_t precision = 0) const {
        std::string repr = (nom.negative() ? "-": "");
        BigInteger lhs(nom.absVal()), rhs(denom.absVal()), z;
        bool divides = false;
        unsigned n = lhs.length();
        for (size_t i = 0; i < n; ++i) {
            z = z * 10 + lhs.getDigit(n - i - 1);
            if (divides || z >= rhs) {
                divides = true;
                int k = 0;
                while (z >= rhs) {
                    z -= rhs;
                    ++k;
                }
                repr += static_cast<char>(k + '0');
            }
        }
        if (!divides) {
            repr += '0';
        }
        repr += '.';
        for (size_t i = 0; i < precision; ++i) {
            z *= 10;
            int k = 0;
            while (z >= rhs) {
                z -= rhs;
                ++k;
            }
            repr += static_cast<char>(k + '0');
        }
        return repr;
    }
    std::string toString() const {
        return nom.toString() + (denom == 1 ? "" :  "/" + denom.toString());
    }
    friend std::istream& operator>>(std::istream& in, Rational& x);
//--------------------------------------------------------------------------
// Logical operators
    bool operator==(const Rational& rhs) const {
        return (nom == rhs.nom) && (denom == rhs.denom);
    }
    bool operator!=(const Rational& rhs) const {
        return !(*this == rhs);
    }
    bool operator<(const Rational& rhs) const {
        return nom * rhs.denom < denom * rhs.nom;
    }
    bool operator>(const Rational& rhs) const {
        return rhs < *this;
    }
    bool operator<=(const Rational& rhs) const {
        return !(*this > rhs);
    }
    bool operator>=(const Rational& rhs) const {
        return !(*this < rhs);
    }
//--------------------------------------------------------------------------
// Arithmetical operators
    const Rational operator-() const {
        return Rational(-nom, denom);
    }
// Modifying
    Rational& operator+=(const Rational& rhs) {
        nom = nom * rhs.denom + denom * rhs.nom;
        denom *= rhs.denom;
        reduce_();
        return *this;
    }
    Rational& operator-=(const Rational& rhs) {
        nom = nom * rhs.denom - denom * rhs.nom;
        denom *= rhs.denom;
        reduce_();
        return *this;
    }
    Rational& operator*=(const Rational& rhs) {
        nom *= rhs.nom;
        denom *= rhs.denom;
        reduce_();
        return *this;
    }
    Rational& operator/=(const Rational& rhs) {
        nom *= rhs.denom;
        denom *= rhs.nom;
        reduce_();
        return *this;
    }
};

// I/O
std::istream& operator>>(std::istream& in, Rational& x) {
    in >> x.nom >> x.denom;
    return in;
}
std::ostream& operator<<(std::ostream& out, const Rational& x) {
    out << x.toString();
    return out;
}
//--------------------------------------------------------------------------
// Arithmetical operators (non-modifying)
const Rational operator+(const Rational& lhs, const Rational& rhs) {
    return Rational(lhs) += rhs;
}
const Rational operator-(const Rational& lhs, const Rational& rhs) {
    return Rational(lhs) -= rhs;
}
const Rational operator*(const Rational& lhs, const Rational& rhs) {
    return Rational(lhs) *= rhs;
}
const Rational operator/(const Rational& lhs, const Rational& rhs) {
    return Rational(lhs) /= rhs;
}
//--------------------------------------------------------------------------
