#define _CRT_DISABLE_PERFCRIT_LOCKS
#define _ CRT_SECURE_NO_WARNINGS
//#define _USE_MATH_DEFINES

/*Allocation control*/
//#define CUSTOM_VECTOR
//#define CUSTOM_ALLOCATOR

/*Hidden features*/
//#define BUILTIN_TREE
//#define BINARY_IO
//#define INT_128

#define DEBUG

#include <bits/stdc++.h>
using namespace std;

/*MACROS*/
//FUNCTIONS
#define in_range(i, l, r) for(ll i = l; i < r; i++)

#define all(v) begin(v), end(v)
#define rall(v) (v).rbegin(), (v).rend()

//#define tr(it, container) for(auto it = begin(container); it != end(container); it++)
#define rtr(it, container) for(auto it = (container).rbegin(); it != (container).rend(); it++)

#define present(element, container) ((container).find(element) != end(container))

#define _T(...) __VA_ARGS__
#define forever() for(;;)

//ABBREVIATIONS
#define sz(c) (ll(c.size()))
#define pb push_back
#define fst first
#define scd second
#define cmpBy(T, field) [](const T& x, const T& y){ return x.field < y.field; }
template<class T> T peek(queue<T>& q) { T top_el = q.front(); q.pop(); return top_el; }

//TYPE SAFETY
#define sqrt(x) sqrt(1.0*(x))
#define pow(x, n) pow(1.0*(x), n)

//CONSTANTS
#define INF (numeric_limits<int>::max())
#define MINF (numeric_limits<int>::min())
#define LINF (numeric_limits<long long>::max())
#define LMINF (numeric_limits<long long>::min())
#define EPS (1E-9)
#define PI ((long double)3.1415926535897932384626433832795028841971693993751)

#define reunique(container) ((container).resize(unique(all(container))-begin(container)))

/*TYPES*/
typedef unsigned long long ull;
typedef long long ll;
typedef pair<int, int> pii;
typedef pair<ll, ll> pll;
typedef long double ld;
typedef vector<vector<int>> graph;

/*TEMPLATE FUNCTIONS*/
//BIT OPERATIONS
template<typename T> inline int bit(T n, T i){return ((n >> i) & T(1));}
inline int bitcount(int v){ v = v - ((v >> 1) & 0x55555555); v = (v & 0x33333333) + ((v >> 2) & 0x33333333); return((v + ((v >> 4) & 0xF0F0F0F)) * 0x1010101) >> 24; }
inline int bitcount(ll v){ int t = v >> 32; int p = (v & ((1LL << 32) - 1)); return bitcount(t) + bitcount(p); }
unsigned int reverse_bits(register unsigned int x){ x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1)); x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2)); x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4)); x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8)); return((x >> 16) | (x << 16)); }
template<typename T> inline bool isPowerOfTwo(T n){return (n != 0 and ((n&(n-1)) == 0)); }
inline int binlog(int n){ assert(n > 0); return 32-__builtin_clz(n)-1; }
inline int binlog(ll n) { assert(n > 0); return 64-__builtin_clzll(n)-1;  }

void bitprint(int n, int w=32){ for (int i = w-1; i >= 0; i--) { std::cout << bit(n, i); } std::cout << "\n"; }
void bitprint(ll n, int w=64){ for (ll i = w-1; i >= 0; i--) { std::cout << bit(n, i); } std::cout << "\n"; }

/*TEMPLATE ALGORITHMS*/
template<typename T> inline T sqr(T x){T x_ = (x); return x_*x_;}
template<typename T> inline T qbr(T x){T x_ = (x); return ((x_*x_)*x_);}
template<typename T> inline int sign(T x){T x_ = (x); return ((x_>T(0))-(x_<T(0)));}
template<typename T> inline T mod(T x, T m){T x_ = (x); return (((x_) >= 0) ? ((x_) % (m)) : ((((x_) % (m))+(m))%(m)));}
template<typename T> inline T gcd(T a, T b){while(b){T t = a % b; a = b; b = t;} return a;}
template<typename T> inline T gcd_ex(T a, T b, T& x, T& y){if(b==0){x=1,y=0; return a;}T x1, y1;T d=gcd_ex(b,a%b,x1,y1);x = y1;y = x1-(a/b)*y1;return d;}
template<typename T> inline T lcm(T a, T b){return (a*(b/gcd(a, b)));}
template<typename A, typename B, typename C>
function<C(A)> combine(function<B(A)> f, function<C(B)> g) {
    return bind(g, bind(f, placeholders::_1));
}
template<typename T> inline bool between(T x, T l, T r) { return l <= x && x <= r; }

//STL INPUT
void fastIO(){ ios::sync_with_stdio(false); std::cin.tie(nullptr); }
template<class T1, class T2> istream& operator >>(istream& in, pair<T1, T2>& P){in >> P.fst >> P.scd; return in;}
template<class T> istream& operator >>(istream& in, vector<T>& Col){for(auto &el : Col) in >> el; return in;}
template<class T> inline void getarr(T* arr, size_t l, size_t r) { in_range(i, l, r) std::cin >> arr[i]; }

//STL OUTPUT
template<class T1, class T2> ostream& operator <<(ostream& os, const pair<T1, T2>& p){os << "(" << p.fst << ", " << p.scd << ")"; return os;}
template<class T> ostream& operator <<(ostream& os, const vector<vector<T>>& v){for(auto &row : v){ for(auto &el : row) os << el << " "; os << "\n";} return os;}
template<class T> ostream& operator <<(ostream& os, const vector<T>& Col){for(auto &el : Col) os << el << " "; return os;}
template<class T> ostream& operator <<(ostream& os, const std::set<T>& Col){for(auto &el : Col) os << el << " "; return os;}
template<class T1, class T2> ostream& operator <<(ostream& os, const map<T1, T2>& Col){for(auto &el : Col) os << el << " "; return os;}
template<class T> inline void printarr(T* arr, size_t l, size_t r) { in_range(i, l, r) {std::cout << arr[i] << " ";}; std::cout << "\n"; }

//VARIADIC INPUT
template<typename First> void read(First& t){ std::cin >> t; }
template<typename First, typename... Args> void read(First& f, Args&... args){ std::cin >> f; read(forward<Args&>(args)...); }

//VARIADIC OUTPUT
template<typename T> void print(T&& t){ std::cout << t << "\n"; }
template<typename First, typename... Args> void print(First&& f, Args&&... args){ std::cout << f << " "; print(forward<Args&&>(args)...); }
template<typename T> void printLn(T&& t){ std::cout << t << "\n"; }
template<typename First, typename... Args> void printLn(First&& f, Args&&... args){ std::cout << f << "\n"; printLn(forward<Args&&>(args)...); }

//VARIADIC TYPE DECLARATION
template <typename T, size_t N> struct MakeTensor{ template <typename... Args> static auto tensor(size_t first, Args... sizes) -> vector<decltype(MakeTensor<T, N-1>::tensor(sizes...))> { auto inner = MakeTensor<T, N-1>::tensor(sizes...); return vector<decltype(inner)>(first, inner);} };
template <typename T> struct MakeTensor<T, 1> { static vector<T> tensor(size_t size) { return vector<T>(size); }};
template <typename T, typename... Args> auto tensor(Args... args) -> decltype(MakeTensor<T, sizeof...(Args)>::tensor(args...)){ return MakeTensor<T, sizeof...(Args)>::tensor(args...); }

//DEBUG
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL_((0,__VA_ARGS__, 5,4,3,2,1))
#define VA_NUM_ARGS_IMPL_(tuple) VA_NUM_ARGS_IMPL tuple
#define VA_NUM_ARGS_IMPL(_0,_1,_2,_3,_4,_5,N,...) N
#define macro_dispatcher(macro, ...) macro_dispatcher_(macro, VA_NUM_ARGS(__VA_ARGS__))
#define macro_dispatcher_(macro, nargs) macro_dispatcher__(macro, nargs)
#define macro_dispatcher__(macro, nargs) macro_dispatcher___(macro, nargs)
#define macro_dispatcher___(macro, nargs) macro ## nargs
#define DBN1(a)         cerr<<#a<<" = "<<(a)<<"\n"
#define DBN2(a,b)       cerr<<#a<<" = "<<(a)<<", "<<#b<<" = "<<(b)<<"\n"
#define DBN3(a,b,c)     cerr<<#a<<" = "<<(a)<<", "<<#b<<" = " <<(b)<<", "<<#c<<" = "<<(c)<<"\n"
#define DBN4(a,b,c,d)   cerr<<#a<<" = "<<(a)<<", "<<#b<<" ="<<(b)<<", "<<#c<<" = "<<(c)<<", "<<#d<<"="<<(d)<<"\n"
#define DBN5(a,b,c,d,e) cerr<<#a<<" = "<<(a)<<", "<<#b<<" = "<<(b)<<", "<<#c<<" = "<<(c)<<", "<<#d<<"="<<(d)<<", "<<#e<<"="<<(e)<<"\n"
#define DBN(...) macro_dispatcher(DBN, __VA_ARGS__)(__VA_ARGS__)

#ifdef CUSTOM_VECTOR
template <class T>
    struct custom_vector : vector<T> {
    custom_vector() : vector<T>() { }
    custom_vector( int n ) : vector<T>(n) { }
    custom_vector( int n, T x ) : vector<T>(n, x) { }
    T &operator [] ( size_t i ) {
        return vector<T>::at(i);
    }
    const T &operator [] ( size_t i ) const {
        return vector<T>::at(i);
    }
};

#define vector custom_vector
#endif


class Treap {
private:
    enum ORDER_T {NONE, ASCENDING, DESCENDING};
    struct Node {
        Node *tl, *tr, *tp;
        long long key;  // value stored in the node
        long long sum;  // sum of values in the subtree
        int prior; // priority (to maintain heap-invariant)
        size_t cnt;   // implicit key
        long long leftmost, rightmost; // the first and the last values in a segment represented by subtree
        long long lazy_add, lazy_assign; // lazy updates
        bool reverse_flag, assign_flag; // lazy updates
        bool is_ascending, is_descending; // ordering in a segment represented by subtree

        explicit Node (long long val) {
            key = val;
            prior = rand();
            cnt = 1;
            sum = val;
            lazy_add = 0;
            tl = tr = tp = nullptr;
            reverse_flag = false;
            assign_flag = false;
            is_ascending = is_descending = true;
            leftmost = rightmost = val;
        }
    };

    typedef Node *pnode;

    // simplifies handling of nullptr corner case
    static inline size_t cnt_ (pnode &T) {
        return T ? T->cnt : 0;
    }

    static inline long long sum_ (pnode &T) {
        return T ? T->sum : 0;
    }

    static inline bool is_ascending_ (pnode &T) {
        return T ? T->is_ascending : false;
    }

    static inline bool is_descending_ (pnode &T) {
        return T ? T->is_descending : false;
    }

    static void push_ (pnode &T) {
        if (!T) { return; }
        if (T->assign_flag) {
            T->key = T->lazy_assign;
            T->leftmost = T->rightmost = T->lazy_assign;
            T->sum = T->cnt * T->lazy_assign;
            if (T->tl) {
                T->tl->assign_flag = true;
                T->tl->lazy_assign = T->lazy_assign;
                T->tl->lazy_add = 0;
            }
            if (T->tr) {
                T->tr->assign_flag = true;
                T->tr->lazy_assign = T->lazy_assign;
                T->tr->lazy_add = 0;
            }
            T->assign_flag = false;
            T->is_ascending = T->is_descending = true;
        }
        if (T->lazy_add) {
            T->key += T->lazy_add;
            T->leftmost += T->lazy_add;
            T->rightmost += T->lazy_add;
            T->sum += T->cnt * T->lazy_add;
            if (T->tl) {
                T->tl->lazy_add += T->lazy_add;
            }
            if (T->tr) {
                T->tr->lazy_add += T->lazy_add;
            }
            T->lazy_add = 0;
        }
        if (T->reverse_flag) {
            if (T->tl) { T->tl->reverse_flag ^= 1; }
            if (T->tr) { T->tr->reverse_flag ^= 1; }
            if (is_ascending_(T) || is_descending_(T)) {
                if (is_ascending_(T) && !is_descending_(T)) { // strictly ascending
                    T->is_ascending = false;
                    T->is_descending = true;
                } else if (!is_ascending_(T) && is_descending_(T)) { // strictly descending
                    T->is_ascending = true;
                    T->is_descending = false;
                } else {
                    // all elements in the range are equal,
                    // do nothing
                }
            }
            std::swap(T->tl, T->tr);
            std::swap(T->leftmost, T->rightmost);
            T->reverse_flag = false;
        }
    }

    static void display_ (pnode T) {
        if (!T) { return; }
        push_(T);
        display_(T->tl);
        std::cout << T->key << " ";
        display_(T->tr);
    }

    static void update_ (pnode &T) {
        if (T) {
            T->cnt = 1 + cnt_(T->tl) + cnt_(T->tr);
            if (T->tl) { T->tl->tp = T; }
            if (T->tr) { T->tr->tp = T; }
            push_(T->tl);
            push_(T->tr);
            T->leftmost = T->tl ? T->tl->leftmost : T->key;
            T->rightmost = T->tr ? T->tr->rightmost : T->key;
            T->sum = T->key + sum_(T->tl) + sum_(T->tr);
            if (!T->tl && !T->tr) {
                T->is_ascending = T->is_descending = true;
            } else if (!T->tl || !T->tr) {
                T->is_ascending = T->tl
                                  ? (T->tl->is_ascending && T->tl->rightmost <= T->key)
                                  : (T->tr->is_ascending && T->key <= T->tr->leftmost);
                T->is_descending = T->tl
                                   ? (T->tl->is_descending && T->key <= T->tl->rightmost)
                                   : (T->tr->is_descending && T->tr->leftmost <= T->key);
            } else {
                T->is_ascending = T->tl->is_ascending && T->tr->is_ascending
                                  && T->tl->rightmost <= T->key
                                  && T->key <= T->tr->leftmost;
                T->is_descending = T->tl->is_descending && T->tr->is_descending
                                   && T->key <= T->tl->rightmost
                                   && T->tr->leftmost <= T->key;
            }
        }
    }

    static pnode merge_ (pnode &L, pnode &R) {
        push_(L);
        push_(R);
        if (!L || !R) {
            return L ? L : R;
        }
        if (L->prior < R->prior) {
            L->tp = R, R->tp = nullptr;
            R->tl = merge_(L, R->tl);
            update_(R);
            return R;
        } else {
            L->tp = nullptr, R->tp = L;
            L->tr = merge_(L->tr, R);
            update_(L);
            return L;
        }
    }

    static void split_ (pnode root, size_t k, pnode &L, pnode &R) {
        if (!root) {
            L = R = nullptr;
            return;
        }
        push_(root);
        root->tp = nullptr;
        if (k < cnt_(root->tl) + 1) {
            split_(root->tl, k, L, root->tl);
            R = root;
        } else {
            split_(root->tr, k - cnt_(root->tl) - 1, root->tr, R);
            L = root;
        }
        update_(root);
    }

    static pnode find_ (pnode root, size_t pos) {
        if (!root) {
            return nullptr;
        }
        push_(root);
        pnode result;
        size_t root_pos = cnt_(root->tl) + 1;
        if (pos < root_pos) {
            result = find_(root->tl, pos);
        } else if (pos == root_pos) {
            result = root;
        } else {
            result = find_(root->tr, pos - root_pos);
        }
        update_(root);
        return result;
    }

    void build_ (pnode& root, long long* data, size_t n) { // it's assumed, that elements of a come in ascending order
        pnode largestNode = nullptr;
        for (size_t i = 0; i < n; ++i) {
            pnode T = new Node(data[i]);
            if (!largestNode) {
                // новая вершина образует дерево
                root = largestNode = T;
            } else if (T->prior <= largestNode->prior) {
                // новая подвешивается справа к последней
                largestNode->tr = T;
                T->tp = largestNode;
                largestNode = T;
            } else {
                pnode curr = largestNode;
                while (curr && curr->prior < T->prior) {
                    curr = curr->tp;
                }
                if (!curr) {
                    // новая вершина становится корнем
                    T->tl = root;
                    root->tp = T;
                    largestNode = T;
                    root = T;
                } else {
                    // новая вершина подвешивается к какой-то
                    // на пути от крайней правой до корня
                    T->tp = curr;
                    pnode tmp = curr->tr;
                    curr->tr = T;
                    T->tl = tmp;
                    tmp->tp = T;
                    largestNode = T;
                }
            }
            update_(T);
            while (T != root) {
                T = T->tp;
                update_(T);
            }
        }
    }

    template<typename Lambda>
    static void apply_on_subsegment_(pnode& root, size_t l, size_t r, Lambda&& func) {
        pnode L, M, R;
        split_(root, l - 1, L, R);
        split_(R, r - l + 1, M, R);
        func(M);
        R = merge_(M, R);
        root = merge_(L, R);
    }

    static void insert_ (pnode &root, long long val, size_t pos) {
        pnode L, M, R;
        split_(root, pos - 1, L, R);
        M = new Node(val);
        L = merge_(L, M);
        root = merge_(L, R);
    }

    static void erase_ (pnode &root, size_t pos) {
        if (!root || pos < 0 || pos > cnt_(root)) {
            return;
        }
        pnode L, M, R;
        split_(root, pos - 1, L, R);
        split_(R, 1, M, R);
        delete M;
        M = nullptr;
        root = merge_(L, R);
    }

    static void reverse_ (pnode &root, size_t l, size_t r) {
        apply_on_subsegment_(root, l, r, [](pnode& vertex) {
            vertex->reverse_flag ^= 1;
        });
    }

    static void assign_ (pnode &root, long long val, size_t l, size_t r) {
        apply_on_subsegment_(root, l, r, [=](pnode& vertex) {
            vertex->assign_flag = true;
            vertex->lazy_assign = val;
            vertex->lazy_add = 0;
        });
    }

    static void add_ (pnode &root, long long val, size_t l, size_t r) {
        apply_on_subsegment_(root, l, r, [=](pnode& vertex) {
           vertex->lazy_add += val;
        });
    }

    static long long get_sum_ (pnode &root, size_t l, size_t r) {
        long long answ = 0;
        apply_on_subsegment_(root, l, r, [&](pnode& vertex) {
            answ = sum_(vertex);
        });
        return answ;
    }

    static void replace_ (pnode &root, size_t pos, const long long &new_val) {
        if (!root) {
            return;
        }
        push_(root);
        size_t root_pos = cnt_(root->tl) + 1;
        if (pos < root_pos) {
            replace_(root->tl, pos, new_val);
        } else if (pos == root_pos) {
            root->key = new_val;
        } else {
            replace_(root->tr, pos - root_pos, new_val);
        }
        update_(root);
    }

    static size_t longest_ordered_suffix_start_pos_ (pnode& root, ORDER_T order) {
        if (!root) {
            return -1;
        }
        push_(root->tl);
        push_(root->tr);
        size_t result;
        size_t curr_pos = cnt_(root->tl) + 1;
        if (order == DESCENDING && root->is_descending
            || order == ASCENDING && root->is_ascending) {
            result = 1;
        } else if (root->tr && ((order == ASCENDING && (!root->tr->is_ascending || root->tr->leftmost < root->key))
                                || (order == DESCENDING && (!root->tr->is_descending || root->tr->leftmost > root->key)))) {
            // уйти вправо
            result = curr_pos + longest_ordered_suffix_start_pos_(root->tr, order);
        } else if (!root->tl || (order == DESCENDING && (root->tl->rightmost < root->key))
                   || (order == ASCENDING && (root->tl->rightmost > root->key))) {
            // остаться на месте
            result = curr_pos;
        } else {
            // уйти влево
            result = longest_ordered_suffix_start_pos_(root->tl, order);
        }
        update_(root);
        return result;
    }

    static size_t predecessor_pos_ (pnode root, const long long& curr, ORDER_T order, long long& prev) {
        size_t result = 0;
        push_(root);
        while (root) {
            if ((order == DESCENDING && curr < root->key)
                || (order == ASCENDING && curr > root->key)) {
                prev = root->key;
                result += cnt_(root->tl) + 1;
                root = root->tr;
            } else {
                root = root->tl;
            }
            push_(root);
        }
        return result;
    }

    static void reorder_suffix_(pnode& root, size_t l, size_t r, ORDER_T new_order) {
        if (l >= r) {
            return;
        }
        ORDER_T curr_order = (new_order == ASCENDING) ? DESCENDING
                                                      : ASCENDING;
        pnode L, M, R;
        split_(root, l - 1, L, R);
        split_(R, r - l + 1, M, R);
        // localize the longest decreasing suffix
        size_t ordered_suff_start_pos = longest_ordered_suffix_start_pos_(M, curr_order);
        pnode suff;
        split_(M, ordered_suff_start_pos - 1, M, suff);
        size_t sufflen = cnt_(suff);
        if (ordered_suff_start_pos <= 1) {
            suff->reverse_flag ^= 1;
        } else {
            // find_ previous element, if any
            // swap them
            long long curr_val = M->rightmost, pred_val;
            size_t prev_in_order_pos = predecessor_pos_(suff, curr_val, curr_order, pred_val);
            // reverse_flag the longest decreasing suffix
            replace_(M, cnt_(M), pred_val);
            replace_(suff, prev_in_order_pos, curr_val);
            suff->reverse_flag ^= 1;
        }
        M = merge_(M, suff);
        R = merge_(M, R);
        root = merge_(L, R);
    }
    static void next_permutation_ (pnode &root, size_t l, size_t r) {
        ORDER_T new_order = ASCENDING;
        reorder_suffix_(root, l, r, new_order);
    }

    static void prev_permutation_ (pnode &root, size_t l, size_t r) {
        ORDER_T new_order = DESCENDING;
        reorder_suffix_(root, l, r, new_order);
    }

    static void deallocate_memory_ (pnode &root) {
        if (!root) {
            return;
        }
        deallocate_memory_(root->tl);
        deallocate_memory_(root->tr);
        delete root;
        root = nullptr;
    }

    pnode root_;
    unsigned size_;

public:
    Treap () {
        root_ = nullptr;
        size_ = 0;
    }
    Treap (long long* data, size_t n) {
        build_(root_, data, n);
        size_ = n;
    }
    ~Treap () {
        deallocate_memory_(root_);
    }

    int size() const {
        return size_;
    }
    void insert(long long val, size_t pos) {
        insert_(root_, val, pos + 1);
        ++size_;
    }
    void erase(size_t pos) {
        erase_(root_, pos + 1);
        --size_;
    }
    void add(long long val, size_t l, size_t r) {
        add_(root_, val, l + 1, r + 1);
    }
    void assign(long long val, size_t l, size_t r) {
        assign_(root_, val, l + 1, r + 1);
    }
    long long sum(size_t l, size_t r) {
        return get_sum_(root_, l + 1, r + 1);
    }
    void next_permutation(size_t l, size_t r) {
        next_permutation_(root_, l + 1, r + 1);
    }
    void prev_permutation(size_t l, size_t r) {
        prev_permutation_(root_, l + 1, r + 1);
    }
    void display() {
        display_(root_);
        std::cout << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    std::cin.tie(0);
//    freopen("in", "r", stdin);
//    freopen("out", "w", stdout);
    srand(time(0));
    Treap* root = nullptr;
    size_t n;
    std::cin >> n;
    if (n) {
        long long* data = new long long[n];
        for (size_t i = 0; i < n; ++i) {
            std::cin >> data[i];
        }
        root = new Treap(data, n);
        delete[] data;
    } else {
        root = new Treap();
    }
    size_t Q;
    std::cin >> Q;
    while (Q--) {
        size_t type;
        std::cin >> type;
        if (type == 1) {
            size_t l, r;
            std::cin >> l >> r;
            long long t_sum = root->sum(l, r);
            std::cout << t_sum << "\n";
        } else if (type == 2) {
            long long val;
            int pos;
            std::cin >> val >> pos;
            root->insert(val, pos);
        } else if (type == 3) {
            int pos;
            std::cin >> pos;
            root->erase(pos);
        } else if (type == 4) {
            int val, l, r;
            std::cin >> val >> l >> r;
            root->assign(val, l, r);
        } else if (type == 5) {
            long long val;
            size_t l, r;
            std::cin >> val >> l >> r;
            root->add(val, l, r);
        } else if (type == 6) {
            size_t l, r;
            std::cin >> l >> r;
            root->next_permutation(l, r);
        } else if (type == 7) {
            size_t l, r;
            std::cin >> l >> r;
            root->prev_permutation(l, r);
        }
    }
    root->display();
    return 0;
}

#ifdef CUSTOM_ALLOCATOR
const int MAX_MEM = 2e8;
int mpos = 0;
char mem[MAX_MEM];
inline void * operator new ( size_t n ) {
    char *res = mem + mpos;
    mpos += n;
    assert(mpos <= MAX_MEM);
    return (void *) res;
}
inline void operator delete ( void * ) { }
#endif // CUSTOM_ALLOCATOR