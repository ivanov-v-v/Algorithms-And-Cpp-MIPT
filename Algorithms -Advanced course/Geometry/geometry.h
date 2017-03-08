#define _USE_MATH_DEFINES
#include <iostream>
#include <utility>      // pair<>
#include <vector>
#include <cmath>        // sqrt(), cos(), sin()
#include <typeinfo>     // typeid()
#include <algorithm>    // copy(), swap()
#include <iterator>     // back_inserter()
#include <stdexcept>    // logic_error()
#include <type_traits>  // is_arithmetic<>

template<typename T>
inline T sqr(const T& a) {
    return a * a;
}
enum signT {minus, plus};
template<typename T>
inline signT sign(const T& a) {
    return a < 0 ? minus : plus;
}

// count elements of parameters pack
template<typename... Args>
constexpr std::size_t length(Args... args) {
    return sizeof...(args);
}

// precision (and comparisons based on it)
const double EPS = 1e-6;
bool equal(double val, double to) {
    return fabs(val - to) < EPS;
}
bool unequal(double val, double to) {
    return !equal(val, to);
}
bool less(double val, double to) {
    return val + EPS < to;
}
bool greater(double val, double to) {
    return less(to, val);
}
bool greaterOrEqual(double val, double to) {
    return !(less(val, to));
}
bool lessOrEqual(double val, double to) {
    return !(greater(val, to));
}

class Line;
struct Point {
    double x, y;
// Constructors
    Point(){}
    Point(double x, double y): x(x), y(y) {}
    Point(const Point& another): x(another.x), y(another.y) {}
    Point& operator =(const Point& another) {
        x = another.x;
        y = another.y;
        return *this;
    }

// Characteristics
    double length() const {
        return std::hypot(x, y);
    }
    double lengthSquared() const {
        return sqr(x) + sqr(y);
    }
// Metric operators
    double distanceTo(const Point& p) const {
        return (p - *this).length();
    }
    double distanceTo(const Line& l) const;
// Comparison operators
    bool operator ==(const Point& another) const {
        return equal(x, another.x) && equal(y, another.y);
    }
    bool operator !=(const Point& another) const {
        return !(*this == another);
    }
    bool operator <(const Point& another) const {
        return less(x, another.x) || (equal(x, another.x) && less(y, another.y));
    }
    bool operator >(const Point& another) const {
        return another < *this;
    }
    bool operator <=(const Point& another) const {
        return !(*this > another);
    }
    bool operator >=(const Point& another) const {
        return !(*this < another);
    }
// Arithmetical operators
    const Point operator -() const {
        return Point(-x, -y);
    }
    const Point operator +(const Point& another) const {
        return Point(x + another.x, y + another.y);
    }
    Point& operator += (const Point& another) {
        x += another.x;
        y += another.y;
        return *this;
    }
    const Point operator -(const Point& another) const {
        return Point(x - another.x, y - another.y);
    }
    Point& operator -=(const Point& another) {
        x -= another.x;
        y -= another.y;
        return *this;
    }
    const Point operator *(double k) const {
        return Point(x * k, y * k);
    }
    Point& operator *=(double k) {
        x *= k;
        y *= k;
        return *this;
    }
    friend const Point operator *(double k, const Point& p);
    const Point operator /(double k) const {
        return Point(*this) *= 1. / k;
    }
    Point& operator /=(double k) {
        return *this *= 1. / k;
    }
// Vector operators
    double operator*(const Point& another) const { // inner product
        return x * another.x + y * another.y;
    }
    double operator %(const Point& another) const { // skew product
        return x * another.y - y * another.x;
    }
    void normalize() {
        *this /= length();
    }
    const Point normalized() const {
        Point directionalVector(*this);
        if (directionalVector.length()) {
            directionalVector.normalize();
        }
        return directionalVector;
    }
    const Point ort() const {
        return Point(-y, x);
    }
// Motions
    void rotate(double angle) {
        bool counterClockwise = angle > 0;
        if (less(angle, 0)) {
            angle *= -1;
        }
        double cosVal = cos(angle), sinVal = sin(angle);
        double newX = x * cosVal - y * sinVal * (counterClockwise ? 1 : -1);
        double newY = (counterClockwise ? 1 : -1) * x * sinVal + y * cosVal;
        x = newX;
        y = newY;
    }
    void rotateAbout(const Point& center, double angle) {
        *this -= center;
        rotate(angle);
        *this += center;
    }
    void reflex(const Point& center) {
        // reflected point p' = 2 * center - p
        *this = 2 * center - *this;
    }
    void reflex(const Line& l);
// Affine transformations
    void scale(const Point& center, double coefficient) {
        *this -= center;
        *this *= coefficient;
        *this += center;
    }
};

std::ostream& operator<<(std::ostream& out, const Point& p) {
    out << '[' << (equal(p.x, 0) ? 0 : p.x) << ", " << (equal(p.y, 0) ? 0 : p.y) << ']';
    return out;
}
const Point operator *(double k, const Point& p) {
    return p * k;
}
signT rotationSign(const Point& origin, const Point& p1, const Point& p2) {
    return sign((p2 - origin) % (p1 - origin));
}

class Line {
private:
    // Ax + By + C = 0, A^2 + B^2 != 0, A >= 0
    double A_, B_, C_;
    void standardize_() {
        if (less(A_, 0)) {
            A_ *= -1;
            B_ *= -1;
            C_ *= -1;
        }
    }
public:
    Line(){}
    Line(const Point& p1, const Point& p2) {
        A_ = p2.y - p1.y;
        B_ = p1.x - p2.x;
        C_ = p2.x * p1.y - p1.x * p2.y;
        standardize_();
    }
    Line(double k, double b) {
        A_ = k;
        B_ = -1;
        C_ = b;
        standardize_();
    }
    Line(const Point& p, double k) {
        A_ = k;
        B_ = -1;
        C_ = p.y - k * p.x;
        standardize_();
    }
    const Point normalVector() const {
        return Point(A_, B_);
    }            // n = (A, B)
    const Point directionalVector() const {
        return Point(-B_, A_);
    }       // a = (-B, A)
    double shift() const {
        return C_;
    }                        // distance to origin

    bool operator ==(const Line& another) const {
        Point n = normalVector();
        Point otherA = another.directionalVector();
        return equal(n * otherA, 0) && equal(C_, another.C_);
    }
    bool operator !=(const Line& l) const {
        return !(*this == l);
    }

    friend std::ostream& operator<<(std::ostream& out, const Line& l);
};

void Point::reflex(const Line& l) {
    Point n = l.normalVector();
    double projectionLength = (*this * n + l.shift()) / n.length();
    *this -= 2 * projectionLength * (n / n.length());
}
double Point::distanceTo(const Line& l) const {
    Point n = l.normalVector();
    return fabs(*this * n + l.shift()) / n.length();
}

std::ostream& operator <<(std::ostream& out, const Line& l) {
    Point n = l.normalVector();
    if (unequal(n.x, 0)) {
        out << n.x << " x";
        if (unequal(n.y, 0)) {
            out << " + ";
        }
    }
    if (unequal(n.y, 0)) {
        out << n.y << " y";
        if (unequal(l.shift(), 0)) {
            out << " + ";
        }
    }
    if (unequal(l.shift(), 0)) {
        out << l.shift();
    }
    out << " == 0";
    return out;
}

class Shape {
public:
    virtual ~Shape(){};

    virtual bool operator ==(const Shape& another) const = 0;
    virtual bool operator !=(const Shape& another) const = 0;

    virtual double perimeter() const = 0;
    virtual double area() const = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;
    virtual bool containsPoint(const Point& p) const = 0;

    virtual void rotate(Point center, double angle) = 0;
    virtual void reflex(Point center) = 0;
    virtual void reflex(Line axis) = 0;
    virtual void scale(Point center, double coefficient) = 0;
};

class Ellipse : public Shape {
protected:
    // (x - x0)^2 / a^2 + (y - y0)^2 / b^2 == 1,
    // a >= b > 0
    //focuses
    Point f1_, f2_;
    double a_, b_;
private:
public:
    Ellipse(){}
    explicit Ellipse(const Point& f1, const Point& f2, double sumOfDistances)
            :
            f1_(f1),
            f2_(f2),
            a_ (sumOfDistances / 2.),
            b_ (std::sqrt(sqr(a_) - (f2 - f1).lengthSquared() / 4.))
    {}
// Characteristics
    const std::pair<Point, Point> focuses() const {
        return std::make_pair(f1_, f2_);
    }
    const std::pair<Line, Line> directrixes() const {
        if (f1_ == f2_) {
            throw std::logic_error("Directrixes are undefined for a circle.");
        }
        Line focalLine(f1_, f2_);
        Point n = focalLine.normalVector();
        Point directionalVector = n.ort();
        double coefficient = directionalVector.y / directionalVector.x;
        double e = eccentricity();
        Line d1((a_/e) * (f1_ - center()).normalized() + center(), coefficient);
        Line d2((a_/e) * (f2_ - center()).normalized() + center(), coefficient);
        return std::make_pair(d1, d2);
    };
    double eccentricity() const {
        if (equal(a_, b_)) {
            throw std::logic_error("Eccentricity is undefined for a circle.");
        }
        return std::sqrt((sqr(a_) - sqr(b_)) / sqr(a_));
    }
    const Point center() const {
        return (f1_ + f2_) / 2;
    }
    virtual double perimeter() const {
        double h  = sqr(a_ - b_) / sqr(a_ + b_);
        double result = M_PI * (a_ + b_) * (1. + ((3. * h) / (10. + std::sqrt(4. - 3. * h))));
        return result;
    }   // Ramanujan's approximation, error <= h^5
    virtual double area() const {
        return M_PI * a_ * b_;
    }
// Comparing two objects
    bool operator ==(const Shape& another) const {
        const Ellipse* anotherEllipsePtr = dynamic_cast<const Ellipse*>(&another);
        if (!anotherEllipsePtr) {
            return false;
        }
        return f1_ == anotherEllipsePtr->f1_ && f2_ == anotherEllipsePtr->f2_ &&
               equal(a_, anotherEllipsePtr->a_) && equal(b_, anotherEllipsePtr->b_);
    }
    bool operator !=(const Shape& another) const {
        return !(*this == another);
    }
    bool isCongruentTo(const Shape& another) const {
        const Ellipse* ellipsePtr = dynamic_cast<const Ellipse*>(&another);
        if (!ellipsePtr) {
            return false;
        }
        return equal(a_, ellipsePtr->a_) && equal(b_, ellipsePtr->b_);
    }
    bool isSimilarTo(const Shape& another) const {
        const Ellipse* ellipsePtr = dynamic_cast<const Ellipse*>(&another);
        if (!ellipsePtr) {
            return false;
        }
        return equal(a_ * ellipsePtr->b_, b_ * ellipsePtr->a_);
    }
// Operations with objects of other types
    bool containsPoint(const Point& p) const {  // (x - x0)^2 / a^2 + (y - y0)^2 / b^2 <= 1
        Point center_ = center();
        return sqr(p.x - center_.x) / sqr(a_) + sqr(p.y - center_.y) / sqr(b_) <= 1;
    }
// I/O operators
    friend std::ostream& operator<<(std::ostream& out, const Ellipse& someEllipse);
// Motions
    void rotate(Point center, double angle) {
        f1_.rotateAbout(center, angle);
        f2_.rotateAbout(center, angle);
    }
    void reflex(Point center) {
        f1_.reflex(center);
        f2_.reflex(center);
    }
    void reflex(Line line) {
        f1_.reflex(line);
        f2_.reflex(line);
    }
// Affine transformations
    void scale(Point center, double coefficient) {
        f1_.scale(center, coefficient);
        f2_.scale(center, coefficient);
    }
};

std::ostream& operator<<(std::ostream& out, const Ellipse& someEllipse) {
    double aSquared = sqr(someEllipse.a_), bSquared = sqr(someEllipse.b_);
    Point center = someEllipse.center();
    if (unequal(center.x, 0)) {
        out << "(x - " << center.x << ")^2";
    } else {
        out << "x^2";
    }
    if (unequal(aSquared, 1)) {
        out << '\\' << aSquared;
    }
    out << " + ";
    if (unequal(center.y, 0)) {
        out << "(y - " << center.y << ")^2";
    } else {
        out << "y^2";
    }
    if (unequal(bSquared, 1)) {
        out << '\\' << bSquared;
    }
    out << " == 1";
    return out;
}

class Circle : public Ellipse {
public:
    Circle(){}
    explicit Circle(const Point& center, double radius) : Ellipse(center, center, 2. * radius)  {}

    friend std::ostream& operator<<(std::ostream& out, const Circle& someCircle);

    double radius() const {
        return a_;
    }
    double perimeter() const {
        return 2 * M_PI * radius();
    }
    double area() const {
        return M_PI * sqr(radius());
    }
};

std::ostream& operator<<(std::ostream& out, const Circle& someCircle) {
    Point center = someCircle.center();
    if (unequal(center.x, 0)) {
        out << "(x - " << center.x << ")^2";
    } else {
        out << "x^2";
    }
    out << " + ";
    if (unequal(center.y, 0)) {
        out << "(y - " << center.y << ")^2";
    } else {
        out << "y^2";
    }
    out << " == " << sqr(someCircle.radius());
    return out;
}

class Polygon : public Shape {
private:
    std::vector<unsigned> zFunction_(const std::vector<Point>& data) const {
        unsigned n = data.size();
        std::vector<unsigned> z(n);
        for (unsigned i = 1, l = 0, r = 0; i < n; ++i) {
            if (i <= r) {
                z[i] = std::min(z[i - l], r - i + 1);
            }
            while(i + z[i] < n && data[z[i] + i] == data[z[i]]) {
                ++z[i];
            }
            if (r < i + z[i] - 1) {
                l = i;
                r = i + z[i] - 1;
            }
        }
        return z;
    }
    bool isCyclicShift_(const std::vector<Point>& lhs, const std::vector<Point>& rhs) const {
        std::vector<Point> concatenated(lhs.begin(), lhs.end());
        concatenated.push_back(Point(1e20, 1e20)); // delimiter point
        for (size_t i = 0; i < 2; ++i) {
            std::copy(rhs.begin(), rhs.end(), std::back_inserter(concatenated));
        }
        std::vector<unsigned> z = zFunction_(concatenated);
        unsigned n = lhs.size();
        for (size_t i = lhs.size() + 1; i < z.size(); ++i) {
            if (z[i] == n) {
                return true;
            }
        }
        return false;
    }
    bool isIsomorphicTo_(const Polygon& another) const {
        // Complexity: O(N)
        // Two polygons are isomorphic, if one sequence of vertices
        // can be obtained from another by cyclic shift of indices.
        // The problem reduces to checking this.
        // This can be done with an O(N)
        // algorithm, based on Z-function:
        // 1. Concatenate one sequence with itself.
        //    New sequence will contain all cyclic shifts
        //    of initial one as subsequences.
        // 2. Append another sequence to the front of concatenated
        //    and separate them with some delimiter.
        // 3. Calculate Z-function for the result.
        //    If at some position its value equals to length
        //    of the sequence used in (2.), polygon are isomorphic.
        if (verticesCount() != another.verticesCount()) {
            return false;
        }
        return isCyclicShift_(vertices_, another.vertices_) ||
               isCyclicShift_(std::vector<Point>(vertices_.rbegin(), vertices_.rend()), another.vertices_);
    }
    bool checkCongruency_(const std::vector<Point>& vertices, const std::vector<Point>& otherVertices) const {
        int n = verticesCount();
        for (int i = 0; i < n; ++i) {
            bool mayBeCongruent = true;
            for (int j = 0; j < n && mayBeCongruent; ++j) {
                const Point& side = vertices[(i + j + 1) % n] - vertices[(i + j) % n];
                const Point &anotherSide = otherVertices[(j + 1) % n] - otherVertices[j];
                if (equal(side.length(), anotherSide.length())) {
                    double triangleArea = side % (vertices[((i + j - 1) % n + n) % n] - vertices[(i + j) % n]);
                    double anotherTriangleArea = anotherSide % (otherVertices[((j - 1) % n + n) % n] - otherVertices[j]);
                    if (unequal(fabs(triangleArea), fabs(anotherTriangleArea))) {
                        mayBeCongruent = false;
                    }
                }
                else {
                    mayBeCongruent = false;
                }
            }
            if (mayBeCongruent) {
                return true;
            }
        }
        return false;
    }
    bool checkSimilarity_(const std::vector<Point>& vertices, const std::vector<Point>& otherVertices) const {
        int n = verticesCount();
        for (int i = 0; i < n; ++i) {
            bool mayBeSimilar = true;
            double coefficient = -1;
            for (int j = 0; j < n && mayBeSimilar; ++j) {
                const Point &side = vertices[(i + j + 1) % n] - vertices[(i + j) % n];
                const Point &anotherSide = otherVertices[(j + 1) % n] - otherVertices[j];
                if (equal(side.length(), 0) ^ equal(anotherSide.length(), 0)) {
                    mayBeSimilar = false;
                } else if (unequal(anotherSide.length(), 0)) {
                    if (coefficient == -1) {
                        coefficient = side.length() / anotherSide.length();
                    }
                    if (unequal(coefficient, (side.length() / anotherSide.length()))) {
                        mayBeSimilar = false;
                    } else {
                        double triangleArea = (side % (vertices[((i + j - 1) % n + n) % n] - vertices[(i + j) % n])) / 2.;
                        double anotherTriangleArea = (anotherSide % (otherVertices[((j - 1) % n + n) % n] - otherVertices[j])) / 2.;
                        if (unequal(fabs(triangleArea), sqr(coefficient) * fabs(anotherTriangleArea))) {
                            mayBeSimilar = false;
                        }
                    }
                }
            }
            if (mayBeSimilar) {
                return true;
            }
        }
        return false;
    }
protected:
    std::vector<Point> vertices_;
    void init_(const Point& p) {
        vertices_.push_back(p);
    }
    template <class... Args>
    void init_(const Point& p, Args... args) {
        vertices_.push_back(p);
        init_(args...);
    }
    // check, whether two polygons differ only in vertices enumeration
    class compareByAngle {
    public:
        bool operator ()(const Point& p1, const Point& p2) {
            double signedArea = p1 % p2;
            return less(0, signedArea) || (equal(signedArea, 0) && p1 < p2);
        }
    };
public:
    Polygon(){}
    Polygon (const std::vector<Point>& someVertices)
            :
            vertices_(someVertices.begin(), someVertices.end())
    {}
    template<class... Args>
    Polygon (Args... args) {
        init_(args...);
    }

    bool operator ==(const Shape& another) const {
        const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&another);
        if (!polygonPtr) {
            return false;
        }
        return isIsomorphicTo_(*polygonPtr);
    }
    bool operator !=(const Shape& another) const {
        return !(*this == another);
    }

    friend std::ostream& operator <<(std::ostream& out, const Polygon& poly);

    unsigned verticesCount() const {
        return vertices_.size();
    }
    std::vector<Point> getVertices() const {
        return std::vector<Point>(vertices_.begin(), vertices_.end());
    };
    virtual double perimeter() const {
        unsigned n = verticesCount();
        double P = 0;
        for (size_t i = 0; i < n; ++i) {
            P += (vertices_[(i + 1) % n] - vertices_[i]).length();
        }
        return P;
    }
    virtual double area() const {
        unsigned n = verticesCount();
        double S = 0;
        for (size_t i = 0; i < n; ++i) {
            const Point &p1 = vertices_[i], &p2 = vertices_[(i + 1) % n];
            S += (p2.x - p1.x) * (p2.y + p1.y) / 2.;
        }
        return std::fabs(S);
    }

    virtual bool isConvex() const {
        unsigned n = verticesCount();
        if (n <= 2) {
            return true;
        }
        int prevSign = rotationSign(vertices_[0], vertices_[n - 1], vertices_[1]);
        for (size_t i = 1; i < n; ++i) {
            int currSign = rotationSign(vertices_[i], vertices_[i - 1], vertices_[i + 1]);
            if (currSign != prevSign) {
                return false;
            }
            prevSign = currSign;
        }
        return true;
    }
    virtual bool isCongruentTo(const Shape& another) const {
        const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&another);
        if (!polygonPtr) {
            return false;
        }
        int n = verticesCount();
        if (n != polygonPtr->verticesCount()) {
            return false;
        }
        if (unequal(perimeter(), polygonPtr->perimeter()) ||
            unequal(area(), polygonPtr ->area())) {
            return false;
        }
        const std::vector<Point> &otherVertices = polygonPtr->vertices_;
        if (checkCongruency_(vertices_, otherVertices)) {
            return true;
        }
        std::vector<Point> revVertices(vertices_.rbegin(), vertices_.rend());
        return checkCongruency_(revVertices, otherVertices);
    }
    virtual bool isSimilarTo(const Shape& another) const {
        const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&another);
        if (!polygonPtr) {
            return false;
        }
        if (verticesCount() != polygonPtr->verticesCount()) {
            return false;
        }
        int n = verticesCount();
        const std::vector<Point> &otherVertices = polygonPtr->vertices_;
        if (checkSimilarity_(vertices_, otherVertices)) {
            return true;
        }
        std::vector<Point> revVertices(vertices_.rbegin(), vertices_.rend());
        return checkSimilarity_(revVertices, otherVertices);
    }
    bool containsPoint(const Point& p) const {
        // O(N) (ray casting algorithm)
        unsigned intersectionCount = 0;
        unsigned n = verticesCount();
        for (size_t i = 0; i < n; ++i) {
            const Point &p1 = vertices_[i], &p2 = vertices_[(i + 1) % n];
            if (lessOrEqual(std::max(p1.x, p2.x), p.x)  &&
                less(std::min(p1.y, p2.y), p.y)     &&
                lessOrEqual(p.y, std::max(p1.y, p2.y))) {
                ++intersectionCount;
            }
        }
        return intersectionCount % 2;
    }

    void rotate(Point center, double angle) {
        for (size_t i = 0; i < verticesCount(); ++i) {
            vertices_[i].rotateAbout(center, angle);
        }
    }
    void reflex(Point center) {
        for (size_t i = 0; i < verticesCount(); ++i) {
            vertices_[i].reflex(center);
        }
    }
    void reflex(Line axis) {
        for (size_t i = 0; i < verticesCount(); ++i) {
            vertices_[i].reflex(axis);
        }
    }
    void scale(Point center, double coefficient) {
        for (size_t i = 0; i < verticesCount(); ++i) {
            vertices_[i].scale(center, coefficient);
        }
    }
};

std::ostream& operator <<(std::ostream& out, const Polygon& poly) {
    out << '[';
    unsigned n = poly.verticesCount();
    for (size_t i = 0; i < n; ++i) {
        out << poly.vertices_[i];
        if (i + 1 != n) {
            out << ", ";
        }
    }
    out << ']';
    return out;
}

class Rectangle : public Polygon {
public:
    Rectangle(){}
    template<typename NumericType>
    Rectangle(const Point& p1, const Point& p2, NumericType coefficient_): Polygon() {
        if (!std::is_arithmetic<NumericType>::value) {
            throw std::logic_error("The proportionality factor must be a number!");
        }
        double coefficient = static_cast<double>(coefficient_);
        if (coefficient > 1.) {
            coefficient = 1. / coefficient;
        }
        double c = (p2 - p1).length();
        double a = std::sqrt(sqr(c) / (sqr(coefficient) + 1.)); // greater side
        double b = std::sqrt(sqr(c) - sqr(a));                  // smaller side
        double proportion = sqr(coefficient) / (1 + sqr(coefficient));
        Point n = Line(p1, p2).normalVector().normalized();
        double h = std::sqrt(sqr(b) - sqr(proportion * (p2 - p1).length()));
        vertices_ = {
                p1,
                p1 + proportion * (p2 - p1) + h * n,
                p2,
                p2 + proportion * (p1 - p2) - h * n
        };
        if (rotationSign(p1, vertices_[1], p2) == plus) {
            vertices_[1].reflex(Line(p1, p2));
            vertices_[3].reflex(Line(p1, p2));
        }
    }
    template<class... Args>
    Rectangle(Args... args): Polygon(args...) {
        if (length(args...) != 4) {
            throw std::logic_error("Rectangle can have only four vertices!");
        }
    }
    Rectangle(const std::vector<Point>& vertices): Polygon(vertices) {
        if (vertices.size() != 4) {
            throw std::logic_error("Rectangle can have only four vertices!");
        }
    }

    const Point center() const {
        return (vertices_[0] + vertices_[2]) / 2.;
    }
    const std::pair<Line, Line> diagonals() const {
        return std::make_pair(Line(vertices_[0], vertices_[2]), Line(vertices_[1], vertices_[3]));
    };

    friend std::ostream& operator<<(std::ostream& out, const Rectangle& someRectangle);
};

std::ostream& operator<<(std::ostream& out, const Rectangle& someRectangle) {
    for (size_t i = 0; i < 4; ++i) {
        out<< someRectangle.vertices_[i];
        if (i != 3) {
            out << ' ';
        }
    }
    return out;
}

class Square : public Rectangle {
public:
    Square(){}
    Square(const Point& p1, const Point& p2): Rectangle(p1, p2, 1.) {}

    const Circle circumscribedCircle() const {
        Point center = (vertices_[0] + vertices_[2]) / 2.;
        double R = (vertices_[2] - vertices_[0]).length() / 2.;
        return Circle(center, R);
    }
    const Circle inscribedCircle() const {
        Point center = (vertices_[0] + vertices_[2]) / 2.;
        double r = (vertices_[1] - vertices_[0]).length() / 2.;
        return Circle(center, r);
    }
};

class Triangle : public Polygon {
public:
    Triangle(){}
    template<class... Args>
    Triangle(Args... args): Polygon(args...) {
        if (length(args...) != 3) {
            throw std::logic_error("Triangle can have only three vertices!");
        }
    }
    Triangle(const std::vector<Point>& vertices): Polygon(vertices) {
        if (vertices.size() != 3) {
            throw std::logic_error("Triangle can have only three vertices!");
        }
    }

    const Point centroid() const {
        return vertices_[0] / 3. + vertices_[1] / 3. + vertices_[2] / 3.;
    }
    const Point orthocenter() const {
        const Point& A = vertices_[0], B = vertices_[1], C = vertices_[2];
        double aSq = (C - B).lengthSquared();
        double bSq = (A - C).lengthSquared();
        double cSq = (B - A).lengthSquared();
        std::vector<double> barCoord {
                (aSq + bSq - cSq) * (aSq - bSq + cSq),
                (aSq + bSq - cSq) * (-aSq + bSq + cSq),
                (aSq - bSq + cSq) * (-aSq + bSq + cSq)
        };
        double normingFactor = barCoord[0] + barCoord[1] + barCoord[2];
        for (size_t i = 0; i < 3; ++i) {
            barCoord[i] /= normingFactor;
        }
        Point result = barCoord[0] * A +
                       barCoord[1] * B +
                       barCoord[2] * C;
        return result;
    }
    const Circle circumscribedCircle() const {
        const Point& A = vertices_[0], B = vertices_[1], C = vertices_[2];
        double aSq = (C - B).lengthSquared();
        double bSq = (A - C).lengthSquared();
        double cSq = (B - A).lengthSquared();
        std::vector<double> barCoord {
                aSq * (bSq + cSq - aSq),
                bSq * (cSq + aSq - bSq),
                cSq * (aSq + bSq - cSq)
        };
        double normingFactor = barCoord[0] + barCoord[1] + barCoord[2];
        for (size_t i = 0; i < 3; ++i) {
            barCoord[i] /= normingFactor;
        }
        Point center = barCoord[0] * A +
                       barCoord[1] * B +
                       barCoord[2] * C;
        double R = (std::sqrt(aSq) * std::sqrt(bSq) * std::sqrt(cSq)) / (4. * area());
        return Circle(center, R);
    }
    const Circle inscribedCircle() const {
        const Point& A = vertices_[0], B = vertices_[1], C = vertices_[2];
        double a = (C - B).length();
        double b = (A - C).length();
        double c = (B - A).length();
        std::vector<double> barCoord {a, b, c};
        double normingFactor = barCoord[0] + barCoord[1] + barCoord[2];
        for (size_t i = 0; i < 3; ++i) {
            barCoord[i] /= normingFactor;
        }
        Point center = barCoord[0] * A +
                       barCoord[1] * B +
                       barCoord[2] * C;
        double R = (2. * area()) / perimeter();
        return Circle(center, R);
    }
    const Line EulerLine() const {
        return Line(ninePointsCircle().center(), centroid());
    }
    const Circle ninePointsCircle() const {
        Point center = (orthocenter() + circumscribedCircle().center()) / 2.;
        double R = circumscribedCircle().radius() / 2.;
        return Circle(center, R);
    }

    friend std::ostream& operator<<(std::ostream& out, const Triangle& someTriangle);
};

std::ostream& operator<<(std::ostream& out, const Triangle& someTriangle) {
    for (size_t i = 0; i < 3; ++i) {
        out<< someTriangle.vertices_[i];
        if (i != 2) {
            out << ' ';
        }
    }
    return out;
}
