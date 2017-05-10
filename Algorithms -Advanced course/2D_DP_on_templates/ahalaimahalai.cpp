#include <iostream>
#include <vector>
#include <cstddef>
#include <algorithm>
using std::vector;

/*TYPES*/
typedef long long ll;

/*TEMPLATE FUNCTIONS*/
//BIT OPERATIONS
template<typename T> inline int bit(T n, T i){return ((n >> i) & T(1));}

/*TEMPLATE ALGORITHMS*/
template<typename T> inline T mod(T x, T m){T x_ = (x); return (((x_) >= 0) ? ((x_) % (m)) : ((((x_) % (m))+(m))%(m)));}

const static ll MOD = 1e9 + 7;

// ДП по профилю
ll count_tilings(size_t n, size_t m) {
    if (n % 2 && m % 2) {
        return 0;
    }
    if (n < m) {
        std::swap(n, m);
    }
    size_t full_mask = (1<<m) - 1;
    vector<vector<long long>> dp(n, vector<long long>(1<<m));
    vector<vector<char>> reachable(1<<m, vector<char>(1<<m));
    for (size_t mask_from = 0; mask_from <= full_mask; ++mask_from) {
        for (size_t mask_to = 0; mask_to <= full_mask; ++mask_to) {
            if (!(mask_from & mask_to)) {
                size_t zeroblock_len = 0;
                bool no_gaps = true;
                for (size_t i = 0; i < m && no_gaps; ++i) {
                    if (!bit(mask_from, i) && !bit(mask_to, i)) {
                        ++zeroblock_len;
                    } else {
                        if (zeroblock_len & 1) {
                            no_gaps = false;
                        }
                        zeroblock_len = 0;
                    }
                }
                if (zeroblock_len & 1) {
                    no_gaps = false;
                }
                reachable[mask_from][mask_to] = no_gaps;
            }
        }
    }
    dp[0][0] = 1;
    for (size_t i = 1; i < n; ++i) {
        for (size_t mask_to = 0; mask_to <= full_mask; ++mask_to) {
            for (size_t mask_from = 0; mask_from <= full_mask; ++mask_from) {
                if (reachable[mask_from][mask_to]) {
                    dp[i][mask_to] = mod(dp[i][mask_to] + dp[i - 1][mask_from], MOD);
                }
            }
        }
    }
    ll total = 0;
    for (size_t mask = 0; mask <= full_mask; ++mask) {
        total = mod(total + dp[n-1][mask], MOD);
    }
    return total;
}

// вспомогательная функция для calc_tilings_fast
void calc(vector<vector<ll>>& dp, size_t n, size_t m,
          size_t row = 0, size_t col = 0,
          size_t mask = 0, size_t new_mask = 0) {
    if (row == n) {
        return;
    }
    if (col >= m) {
        dp[row + 1][new_mask] = mod(dp[row + 1][new_mask] + dp[row][mask], MOD);
    } else {
        size_t curr_bit = 1ULL << col;
        if (curr_bit & mask) {
            calc(dp, n, m, row, col + 1, mask, new_mask);
        } else {
            calc(dp, n, m, row, col + 1, mask, new_mask | curr_bit);
            if ((col + 1) < m && !bit(mask, col) && !bit(mask, col + 1)) {
                calc(dp, n, m, row, col + 2, mask, new_mask);
            }
        }
    }
}

// ДП по изломанному профилю
ll count_tilings_fast(size_t n, size_t m) {
    if (n % 2 && m % 2) {
        return 0;
    }
    if (n < m) {
        std::swap(n, m);
    }
    vector<vector<ll>> dp(n + 1, vector<ll>(1 << m));
    dp[0][0] = 1;
    size_t full_mask = (1<<m) - 1;
    for (size_t row = 0; row < n; ++row) {
        for (size_t mask = 0; mask <= full_mask; ++mask) {
            calc(dp, n, m, row, 0, mask);
        }
    }
    return dp[n][0];
}

// переходы динамики
template <size_t N, size_t M, size_t pivot = 0, size_t mask = 0, size_t reachable = 1>
class AhalaiMahalai {
public:
    // 1. Клетка профиля занята, перейти к следующей.
    // 2. Положить доминошку вертикально.
    // 3. Положить доминошку горизонтально.
    constexpr static long long value =
            (AhalaiMahalai<N, M, pivot + 1, mask ^ (1ULL << pivot), (mask >> pivot) & 1>::value +
             AhalaiMahalai<N, M, pivot + 1, mask | (1ULL << pivot), !((mask >> pivot) & 1)>::value +
             AhalaiMahalai<N, M, pivot + 1, mask | (1ULL << (pivot + 1)), !((mask >> pivot) & 1) && !((mask >> (pivot + 1)) & 1)>::value) % MOD;
};

// переход в запрещённую конфигурацию, отсечение
template <size_t N, size_t M, size_t pivot, size_t mask>
class AhalaiMahalai<N, M, pivot, mask, 0> {
public:
    constexpr static long long value = 0;
};

// финальная позиция
template <size_t M>
class AhalaiMahalai<0, M, 0, 0, 1> {
public:
    constexpr static long long value = 1;
};

template <size_t M, size_t pivot, size_t mask>
class AhalaiMahalai<0, M, pivot, mask, 1> {
public:
    constexpr static long long value = 0;
};

// переход на новую строку
template <size_t N, size_t M, size_t mask>
class AhalaiMahalai<N, M, M, mask, 1> {
public:
    constexpr static long long value = AhalaiMahalai<N - 1, M, 0, mask, 1>::value;
};


int main() {
    std::cout << AhalaiMahalai<100, 6>::value << std::endl;
    std::cout << count_tilings_fast(100, 6) << std::endl;
    return 0;
}
