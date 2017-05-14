// переходы динамики
template <unsigned M, unsigned N, unsigned pivot = 0, unsigned mask = 0, unsigned reachable = 1>
class AhalaiMahalai {
public:
    // 1. Клетка профиля занята, перейти к следующей.
    // 2. Положить доминошку вертикально.
    // 3. Положить доминошку горизонтально.
    constexpr static long long value =
            (AhalaiMahalai<M, N, pivot + 1, mask ^ (1ULL << pivot), (mask >> pivot) & 1>::value +
             AhalaiMahalai<M, N, pivot + 1, mask | (1ULL << pivot), !((mask >> pivot) & 1)>::value +
             AhalaiMahalai<M, N, pivot + 1, mask | (1ULL << (pivot + 1)), !((mask >> pivot) & 1) && !((mask >> (pivot + 1)) & 1)>::value) % MODULUS;
};

// переход в запрещённую конфигурацию, отсечение
template <unsigned M, unsigned N, unsigned pivot, unsigned mask>
class AhalaiMahalai<M, N, pivot, mask, 0> {
public:
    constexpr static long long value = 0;
};

// финальная позиция
template <unsigned M>
class AhalaiMahalai<M, 0, 0, 0, 1> {
public:
    constexpr static long long value = 1;
};

template <unsigned M, unsigned pivot, unsigned mask>
class AhalaiMahalai<M, 0, pivot, mask, 1> {
public:
    constexpr static long long value = 0;
};

// переход на новую строку
template <unsigned M, unsigned N, unsigned mask>
class AhalaiMahalai<M, N, M, mask, 1> {
public:
    constexpr static long long value = AhalaiMahalai<M, N - 1, 0, mask, 1>::value;
};

