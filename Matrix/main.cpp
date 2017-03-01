#include "headers/rational.h"
#include <bits/stdc++.h>
using namespace std;

/* TODO: make Strassen multiplication at least two times faster.
 * TODO: get rid of constexpression and unnecessary static members.
 * TODO: enhance multiplication of rationals using non-recursive FFT
 * TODO: learn how to write notes in LaTeX (in order to, for example, prepare for exams)
 * */

enum ENumberType {undefined, prime, composite};

template <int N>
class Finite{
private:
    static ENumberType modulusType;
    int value;

    static int mod(int val) {
        return val < 0 ? (val % N + N) % N : val % N;
    }
    static int extendedEuclid(int a, int b, int& x, int& y) {
        if (!b) {
            x = 1, y = 0;
            return a;
        }
        int x1, y1;
        int g = extendedEuclid(b, a % b, x1, y1);
        x = mod(y1);
        y = mod(x1 - (a / b) * y1);
        return g;
    }
    static int modularInverse(int val, int modulus) {
        if (!val) {
            throw std::logic_error("Zero is non-invertible!");
        }
        int x, y;
        int g = extendedEuclid(val, modulus, x, y);
        if (g != 1) {
            throw std::logic_error("Value and modulus are not relatively prime!");
        }
        return x;
    }
    static bool isPrime(int value) {
        for (int i = 2; 1LL * i * i <= value; ++i) {
            if (!(value % i)) {
                return false;
            }
        }
        return true;
    }

public:
    Finite(int x = 0): value(mod(x)) {
        if (modulusType == undefined) {
            modulusType = isPrime(N) ? prime : composite;
        }
    }
    Finite(const Finite& another): value(another.value) {
        if (modulusType == undefined) {
            modulusType = isPrime(N) ? prime : composite;
        }
    }

    explicit operator bool() const {
        return value;
    }

    bool operator ==(const Finite& another) const {
        return !((value - another.value) % N);
    }
    bool operator !=(const Finite& another) const {
        return !(*this == another);
    }

    Finite& operator +=(const Finite& another) {
        value = mod(value + another.value);
        return *this;
    }
    Finite& operator -=(const Finite& another) {
        value = mod(value - another.value);
        return *this;
    }
    Finite& operator *=(const Finite& another) {
        value = mod(value * another.value);
        return *this;
    }
    Finite& operator /=(const Finite& another) {
        if (modulusType != prime) {
            throw logic_error("Composite modulus: division is undefined!");
        }
        if (!another) {
            throw logic_error("Division by zero!");
        }
        value = mod(value * modularInverse(another.value, N));
        return *this;
    }
    Finite operator -() const {
        return Finite(*this) *= -1;
    }

    Finite operator +(const Finite& another) const {
        return Finite(value) += another.value;
    }
    Finite operator -(const Finite& another) const {
        return Finite(value) -= another.value;
    }
    Finite operator *(const Finite& another) const {
        return Finite(value) *= another.value;
    }
    Finite operator /(const Finite& another) const {
        return Finite(value) /= another.value;
    }

    friend istream& operator >>(istream& in, Finite& num) {
        in >> num.value;
        return in;
    }
    friend ostream& operator <<(ostream& out, const Finite& num) {
        out << num.value;
        return out;
    }
};


template<int N>
ENumberType Finite<N>::modulusType = undefined;

constexpr int getNextPow(int n) {
    return !n ? 1 : ~(n & (n - 1)) ? n : (1 << (32 - __builtin_clz(n)));
}

template <unsigned N, unsigned M, typename Field = Rational>
class Matrix;

template <unsigned N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;

template <unsigned N, typename Field = Rational>
using Vector = Matrix<N, 1, Field>;

template <unsigned N, typename Field = Rational>
using Covector = Matrix<1, N, Field>;

template <unsigned N, unsigned M, typename Field>
class Matrix {
private:
    Field** table;

    constexpr static const int boundingBox = getNextPow(N > M ? N : M);
    static void multiplyRow(Field* row, const Field& value) {
        for (size_t i = 0; i < M; ++i) {
            row[i] *= value;
        }
    }
    static void subtractRow(Field* minuend, Field* subtrahend, int fromPos = 0) {
        for (size_t i = fromPos; i < M; ++i) {
            minuend[i] -= subtrahend[i];
        }
    }
    int getPivotRow(int pivotCol, int fromPos = 0) {
        int rowIndex = fromPos;
        for (; rowIndex < N; ++rowIndex) {
            if (table[rowIndex][pivotCol] && !pivotCol || std::all_of(table[rowIndex], table[rowIndex] + pivotCol, [](const Field val) { return !val; })) {
                return rowIndex;
            }
        }
        return rowIndex;
    }
    void performGaussianElimination() {
        for (size_t pivotCol = 0; pivotCol < M; ++pivotCol) {
            int pivotRow = getPivotRow(pivotCol, pivotCol);
            std::swap(table[pivotRow], table[pivotCol]);
            pivotRow = pivotCol;
            for (size_t i = pivotRow + 1; i < N; ++i) {
                if (table[i][pivotCol]) {
                    Field pivotCoeff = table[i][pivotCol] / table[pivotRow][pivotCol];
                    multiplyRow(table[pivotRow], pivotCoeff);
                    subtractRow(table[i], table[pivotRow], pivotCol);
                    multiplyRow(table[pivotRow], Field(1) / pivotCoeff);
                }
            }
        }
    }

    template <unsigned K>
    static SquareMatrix<K, Field> multiplyNaively(SquareMatrix<K, Field>& A, SquareMatrix<K, Field>& B) {
        SquareMatrix<K, Field> res;
        for (size_t i = 0; i < K; ++i) {
            for (size_t j = 0; j < K; ++j) {
                for (size_t k = 0; k < K; ++k) {
                    res[i][j] += A[i][k] * B[k][j];
                }
            }
        }
        return res;
    }

    template<unsigned L>
    static SquareMatrix<L, Field> strassenMultiply(SquareMatrix<L, Field> A, SquareMatrix<L, Field> B) {
        if (L <= 64) {
            SquareMatrix<L, Field> result = multiplyNaively(A, B);
            return result;
        }

        SquareMatrix<L / 2, Field> A11(A, 0, L / 2, 0, L / 2),
                            A12(A, 0, L / 2, L / 2, L),
                            A21(A, L / 2, L, 0, L / 2),
                            A22(A, L / 2, L, L / 2, L);

        SquareMatrix<L / 2, Field> B11(B, 0, L / 2, 0, L / 2),
                            B12(B, 0, L / 2, L / 2, L),
                            B21(B, L / 2, L, 0, L / 2),
                            B22(B, L / 2, L, L / 2, L);

        SquareMatrix<L / 2, Field> C11, C12, C21, C22;
        SquareMatrix<L / 2, Field> M1, M2, M3, M4, M5, M6, M7;

        M1 = strassenMultiply(A11 + A22, B11 + B22);
        M2 = strassenMultiply(A21 + A22, B11);
        M3 = strassenMultiply(A11, B12 - B22);
        M4 = strassenMultiply(A22, B21 - B11);
        M5 = strassenMultiply(A11 + A12, B22);
        M6 = strassenMultiply(A21 - A11, B11 + B12);
        M7 = strassenMultiply(A12 - A22, B21 + B22);

        C11 = M1 + M4 - M5 + M7;
        C12 = M3 + M5;
        C21 = M2 + M4;
        C22 = M1 - M2 + M3 + M6;

        SquareMatrix<L, Field> result;
        for (size_t i = 0; i < L / 2; ++i) {
            for (size_t j = 0; j < L / 2; ++j) {
                result[i][j] = C11[i][j];
                result[i][j + L / 2] = C12[i][j];
                result[i + L / 2][j] = C21[i][j];
                result[i + L / 2][j + L / 2] = C22[i][j];
            }
        }
        return result;
    }
public:
    Matrix() {
        table = new Field*[N]();
        for (size_t i = 0; i < N; ++i) {
            table[i] = new Field[M]();
        }
    }
    Matrix(const Field** data) {
        table = new Field*[N]();
        for (size_t i = 0; i < N; ++i) {
            table[i] = new Field[M];
            for (size_t j  = 0; j < M; ++j) {
                table[i][j] = data[i][j];
            }
        }
    }
    Matrix(const Matrix& another) {
        table = new Field*[N]();
        for (size_t i = 0; i < N; ++i) {
            table[i] = new Field[M];
            std::copy(begin(another[i]), begin(another[i]) + M, table[i]);
        }
    }
    template<unsigned N1, unsigned M1>
    Matrix(const Matrix<N1, M1, Field>& another, int firstRow, int lastRow, int firstCol, int lastCol) {
        static_assert(N1 >= N && M1 >= M,
            "Trying to extract invalid submatrix!"
         );
        table = new Field*[N]();
        for (size_t i = firstRow; i < lastRow; ++i) {
            table[i - firstRow] = new Field[M]();
            std::vector<Field> row = another.getRow(i);
            for (size_t j = firstCol; j < lastCol; ++j) {
                table[i - firstRow][j - firstCol] = row[j];
            }
        }
    }
    ~Matrix() {
        for (size_t i = 0; i < N; ++i) {
            delete[] table[i];
        }
        delete[] table;
    }
    Matrix& operator=(const Matrix& another) {
        if (this != &another) {
            for (size_t i = 0; i < N; ++i) {
                for (size_t j = 0; j < M; ++j) {
                    table[i][j] = another[i][j];
                }
            }
        }
        return *this;
    }

    Field*& operator[](int index) {
        return table[index];
    }

    const Field* operator[](int index) const {
        return table[index];
    }

    std::vector<Field> getRow(unsigned rowIndex) const {
        std::vector<Field> row;
        for (size_t i = 0; i < M; ++i) {
            row.push_back(table[rowIndex][i]);
        }
        return row;
    }
    std::vector<Field> getColumn(unsigned columnIndex) const {
        std::vector<Field> column;
        for (size_t i = 0; i < N; ++i) {
            column.push_back(table[i][columnIndex]);
        }
        return column;
    }

    void inverse() {
        static_assert(N == M, "Non-square matrices are non-invertible!");
        Matrix copy(*this);
        for (size_t i = 0; i < N; ++i) {
            fill(table[i], table[i] + M, Field(0));
        }
        for (size_t i = 0; i < N; ++i) {
            table[i][i] = 1;
        }
        for (size_t pivotCol = 0; pivotCol < M; ++pivotCol) {
            int pivotRow = copy.getPivotRow(pivotCol);
            if (pivotRow == N) {
                break;
            }
            for (size_t i = 0; i < N; ++i) {
                if (i == pivotRow) {
                    continue;
                }
                if (copy[i][pivotCol]) {
                    Field pivotCoeff = copy[i][pivotCol] / copy[pivotRow][pivotCol];

                    multiplyRow(table[pivotRow], pivotCoeff);
                    subtractRow(table[i], table[pivotRow]);
                    multiplyRow(table[pivotRow], Field(1) / pivotCoeff);

                    multiplyRow(copy[pivotRow], pivotCoeff);
                    subtractRow(copy[i], copy[pivotRow]);
                    multiplyRow(copy[pivotRow], Field(1) / pivotCoeff);
                }
            }
        }

        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                if (copy[i][j] && copy[i][j] != 1) {
                    Field normingCoeff = Field(1) / copy[i][j];
                    multiplyRow(table[i], normingCoeff);
                    break;
                }
            }
        }
    }
    const Matrix inverted() const {
        Matrix copy(*this);
        copy.inverse();
        return copy;
    }

    void transpose() {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = i + 1; j < M; ++j) {
                std::swap(table[i][j], table[j][i]);
            }
        }
    }
    const Matrix transposed() const {
        Matrix copy(*this);
        copy.transpose();
        return copy;
    }

    const Field det() const {
        static_assert(N == M, "Determinant is undefined for rectangular matrices!");
        Matrix copy(*this);
        copy.performGaussianElimination();
        Field result = 1;
        for (size_t i = 0; i < N; ++i) {
            result *= copy[i][i];
        }
        return result;
    }
    int rank() const {
        Matrix copy(*this);
        copy.performGaussianElimination();
        int result = 0;
        for (size_t i = 0; i < N && copy[i][i]; ++i) {
            ++result;
        }
        return result;
    }
    const Field trace() const {
        Field result = 0;
        for (size_t i = 0; i < N; ++i) {
            result += table[i][i];
        }
        return result;
    }

    bool operator ==(const Matrix& another) const {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                if (table[i][j] != another[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }
    bool operator !=(const Matrix& another) const {
        return !(*this == another);
    }

    Matrix& operator +=(const Matrix& another) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                table[i][j] += another[i][j];
            }
        }
        return *this;
    }
    Matrix& operator -=(const Matrix& another) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                table[i][j] -= another[i][j];
            }
        }
        return *this;
    }

    template<unsigned K>
    Matrix& operator *=(const Matrix<M, K, Field>& another) {
        typedef SquareMatrix<N < K ? boundingBox : another.boundingBox, Field> StrassenMatrix; // 2^k * 2^k
        StrassenMatrix A;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                A[i][j] = table[i][j];
            }
        }
        StrassenMatrix B;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < K; ++j) {
                B[i][j] = another[i][j];
            }
        }
        StrassenMatrix result = strassenMultiply(A, B);
        for (size_t i = 0; i < N; ++i) {
            std::copy(result[i], result[i] + K, table[i]);
        }
        return *this;
    }

    Matrix operator +(const Matrix& another) const {
        return Matrix(*this) += another;
    }
    Matrix operator -(const Matrix& another) const {
        return Matrix(*this) -= another;
    }
    template <unsigned K>
    Matrix operator *(const Matrix<M, K, Field>& another) const {
        return Matrix(*this) *= another;
    }

    Matrix& operator *=(const Field& scalar) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                table[i][j] *= scalar;
            }
        }
        return *this;
    }
    Matrix operator *(const Field& scalar) const {
        return Matrix(*this) *= scalar;
    }
    friend Matrix operator *(const Field& scalar, const Matrix& someMatrix) {
        return someMatrix * scalar;
    }

    friend std::istream& operator >>(std::istream& in, Matrix& someMatrix) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                in >> someMatrix[i][j];
            }
        }
        return in;
    }
    friend std::ostream& operator <<(std::ostream& out, Matrix& someMatrix) {
        out << "[";
        for (size_t i = 0; i < N; ++i) {
            out << "[";
            for (size_t j = 0; j < M; ++j) {
                out << someMatrix[i][j];
                if (j + 1 != M) {
                    out << ", ";
                }
            }
            out << "]";
            if (i + 1 != N) {
                out << ",\n";
            }
        }
        out << "]";
        return out;
    }
    template<unsigned N1, unsigned M1, typename Field1>
    friend class Matrix;
};

int main () {
    freopen("in", "r", stdin);
//    Matrix<16, 16, double> M1;
//    Matrix<16, 16, double> M2;
//    M1.test();
//    cin >> M1 >> M2;
//    std::cout << M1 << "\n\n";
//    std::cout << M2 << "\n\n";
//    auto t0 = clock();
//    M1 *= M2;
//    cout << (clock() - t0) / double(CLOCKS_PER_SEC);
//    std::cout << M1 << "\n\n" << M1.rank();
////    Matrix<3, 3> InvM = M.inverted();
////    Matrix<3, 3> res = InvM * M;
////    cout << InvM << endl;
    return 0;
}