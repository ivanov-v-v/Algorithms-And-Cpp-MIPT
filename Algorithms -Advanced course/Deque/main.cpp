#include "src/deque.cpp"
#include "headers/timsort.h"
#include "headers/rational.h"
#include <deque>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <gtest/gtest.h>

clock_t BEGIN_T, END_T;
#define SET_TIMER BEGIN_T = clock()
#define STOP_TIMER END_T = clock()
#define MEASURED_T (END_T - BEGIN_T) / static_cast<double>(CLOCKS_PER_SEC)

TEST(BASIC_FUNCTIONALITY, DEFAULT_CONSTRUCTOR) {
    EXPECT_NO_THROW(vector<Deque<int>>(1000));
}

TEST(BASIC_FUNCTIONALITY, COPY_CONSTRUCTOR) {
    Deque<int> some_deque;
    for (size_t i = 0; i < rand() % 10000 + 1; ++i) {
        if (rand() & 1) {
            some_deque.push_front(rand());
        } else {
            some_deque.push_back(rand());
        }
    }
    Deque<int> copy_deque = some_deque;
    ASSERT_EQ(copy_deque, some_deque);
}

TEST(BASIC_FUNCTIONALITY_TESTS, ASSIGNMENT_OPERATOR) {
    Deque<int> some_deque;
    for (size_t i = 0; i < rand() % 10000 + 1; ++i) {
        some_deque.push_back(rand());
    }
    Deque<int> another_deque;
    another_deque = some_deque;
    ASSERT_EQ(some_deque, another_deque);
}

TEST(BASIC_FUNCTIONALITY, DEQUE_OF_DEQUES) {
    Deque<Deque<int>> nested;
    Deque<int> some_deque;
    for (size_t i = 0; i < 5; ++i) {
        ASSERT_NO_THROW(some_deque.push_front(rand()));
    }
    ASSERT_NO_THROW(nested.push_back(some_deque));
    ASSERT_EQ(some_deque, nested.back());
    std::cout << "\n" << std::endl;
}

TEST(BASIC_FUNCTIONALITY, DEQUE_OF_DEQUES_OF_DEQUES) {
    Deque<Deque<Deque<int>>> heavily_nested;
    const size_t DIMENSION = 5;
    for (size_t i = 0; i < DIMENSION; ++i) {
        Deque<Deque<int>> nested;
        for (size_t j = 0; j < DIMENSION; ++j) {
            Deque<int> temp;
            for (size_t k = 0; k < DIMENSION; ++k) {
                if (rand() & 1) {
                    ASSERT_NO_THROW(temp.push_back(rand() * rand() % 100));
                } else {
                    ASSERT_NO_THROW(temp.push_front(rand() * rand() % 100));
                }
            }
            if (rand() & 1) {
                ASSERT_NO_THROW(nested.push_back(temp));
            } else {
                ASSERT_NO_THROW(nested.push_front(temp));
            }
        }
        if (rand() & 1) {
            ASSERT_NO_THROW(heavily_nested.push_back(nested));
        } else {
            ASSERT_NO_THROW(heavily_nested.push_front(nested));
        }
    }
    for (auto hn_it = heavily_nested.cbegin(); hn_it != heavily_nested.cend(); ++hn_it) {
        for (auto n_it = hn_it->cbegin(); n_it != hn_it->cend(); ++n_it) {
            for (auto it = n_it->cbegin(); it != n_it->cend(); ++it) {
                ASSERT_NO_THROW(*it);
                std::cout << *it;
                if (it + 1 != n_it->cend()) {
                    std::cout << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    std::cout << "\n" << std::endl;
}

TEST(BASIC_FUNCTIONALITY, PUSH_AND_POP) {
    const std::vector<int> TEST_LEN({100, 1000, 10000, 100000, 1000000});
    for (size_t TEST_ID = 0; TEST_ID < TEST_LEN.size(); ++TEST_ID) {
        SET_TIMER;
        Deque<int> custom_deque;
        for (size_t i = 0; i < TEST_LEN[TEST_ID]; ++i) {
            if (i & 1) {
                custom_deque.push_back(rand());
            } else {
                custom_deque.push_front(rand());
            }
        }
        ASSERT_EQ(custom_deque.size(), TEST_LEN[TEST_ID]);
        for (size_t i = 0; i < TEST_LEN[TEST_ID]; ++i) {
            if (i & 1) {
                custom_deque.pop_front();
            } else {
                custom_deque.pop_back();
            }
        }
        ASSERT_TRUE(custom_deque.empty());
        STOP_TIMER;
        double custom_t = MEASURED_T;

        SET_TIMER;
        std::deque<int> default_deque;
        for (size_t i = 0; i < TEST_LEN[TEST_ID]; ++i) {
            if (i & 1) {
                default_deque.push_back(rand());
            } else {
                default_deque.push_front(rand());
            }
        }
        for (size_t i = 0; i < TEST_LEN[TEST_ID]; ++i) {
            if (i & 1) {
                default_deque.pop_front();
            } else {
                default_deque.pop_back();
            }
        }
        STOP_TIMER;
        double default_t = MEASURED_T;

        EXPECT_LE(custom_t, default_t);

        std::cout << "Push and pop " << TEST_LEN[TEST_ID] << " elements, total time: " <<
            custom_t << " compared to " << default_t << "\n";
    }
    std::cout << "\n" << std::endl;
}

TEST(BASIC_FUNCTIONALITY, BACK_AND_FRONT) {
    Deque<int> custom_deque;
    std::deque<int> default_deque;
    for (size_t test = 0; test < rand() % 10000 + 1; ++test) {
        if (rand() & 1) {
            if (rand() & 1) {
                custom_deque.push_front(rand());
                default_deque.push_front(custom_deque.front());
            } else {
                custom_deque.push_back(rand());
                default_deque.push_back(custom_deque.back());
            }
        } else {
            if (!custom_deque.empty()) {
                if (rand() & 1) {
                    ASSERT_EQ(custom_deque.back(), default_deque.back());
                    custom_deque.pop_back();
                    default_deque.pop_back();
                } else {
                    ASSERT_EQ(custom_deque.front(), default_deque.front());
                    custom_deque.pop_front();
                    default_deque.pop_front();
                }
            } else {
                ASSERT_TRUE(custom_deque.empty() && default_deque.empty());
            }
        }
    }
}

TEST(ITERATOR_TESTS, TRAVERSAL) {
    Deque<double> custom_deque;
    for (size_t i = 0; i < rand() % 100000 + 1; ++i) {
        if (rand() & 1) {
            ASSERT_NO_THROW(custom_deque.push_front(std::sqrt(rand()) * rand()));
        } else {
            ASSERT_NO_THROW(custom_deque.push_back(std::sqrt(rand()) * rand()));
        }
    }
    auto it = custom_deque.cbegin();
    for (size_t i = 0; i < custom_deque.size(); ++i) {
        ASSERT_EQ(*it, *(custom_deque.cbegin() + i));
        ++it;
    }
}

TEST(ITERATOR_TESTS, REVERSED_TRAVERSAL) {
    Deque<double> custom_deque;
    for (size_t i = 0; i < rand() % 100000 + 1; ++i) {
        if (rand() & 1) {
            ASSERT_NO_THROW(custom_deque.push_front(std::sqrt(rand()) * rand()));
        } else {
            ASSERT_NO_THROW(custom_deque.push_back(std::sqrt(rand()) * rand()));
        }
    }
    auto it = custom_deque.crbegin();
    for (size_t i = 0; i < custom_deque.size(); ++i) {
        ASSERT_EQ(*it, *(custom_deque.cbegin() + (custom_deque.size() - i - 1)));
        ++it;
    }
}

TEST(ITERATOR_TESTS, DEQUE_OF_DEQUES) {
    Deque<Deque<int>> custom_deque;
    const size_t OUTER_SIZE = 10;
    const size_t INNER_SIZE = 16;
    for (size_t i = 0; i < OUTER_SIZE; ++i) {
        Deque<int> temp;
        for (size_t j = 0; j < INNER_SIZE; ++j) {
            if (rand() & 1) {
                ASSERT_NO_THROW(temp.push_front(rand() * rand() % 100));
            } else {
                ASSERT_NO_THROW(temp.push_back(rand() * rand() % 100));
            }
        }
        if (rand() & 1) {
            ASSERT_NO_THROW(custom_deque.push_back(temp));
        } else {
            ASSERT_NO_THROW(custom_deque.push_front(temp));
        }
        custom_deque.back().display_storage();
    }
    std::cout << "\n" << std::endl;
}

TEST(PERFORMANCE_TESTS, SORTING_DOUBLE) {
    const size_t DATA_SIZE = 1000000;
    Deque<double> custom_deque;
    std::deque<double> default_deque;
    for (size_t i = 0; i < DATA_SIZE; ++i) {
        custom_deque.push_front(sqrt(rand()) * rand());
        default_deque.push_front(*custom_deque.begin());
    }
    ASSERT_EQ(custom_deque.size(), DATA_SIZE);

    SET_TIMER;
    std::sort(custom_deque.begin(), custom_deque.end());
    STOP_TIMER;
    double custom_sorting_t = MEASURED_T;

    SET_TIMER;
    std::sort(default_deque.begin(), default_deque.end());
    STOP_TIMER;

    double default_sorting_t = MEASURED_T;
    for (size_t i = 0; i < DATA_SIZE - 1; ++i) {
        ASSERT_TRUE(custom_deque[i] <= custom_deque[i + 1]);
    }
    std::cout << "Sorting " << DATA_SIZE << " random real numbers: " <<
          custom_sorting_t << " compared to " << default_sorting_t << "\n";

    SET_TIMER;
    std::sort(custom_deque.rbegin(), custom_deque.rend());
    STOP_TIMER;
    custom_sorting_t = MEASURED_T;

    SET_TIMER;
    std::sort(default_deque.rbegin(), default_deque.rend());
    STOP_TIMER;
    default_sorting_t = MEASURED_T;

    for (size_t i = 0; i < DATA_SIZE - 1; ++i) {
        ASSERT_TRUE(custom_deque[i] >= custom_deque[i + 1]);
    }

    EXPECT_LE(custom_sorting_t, default_sorting_t);

    std::cout << "Sorting " << DATA_SIZE << " random real numbers in reversed order: " <<
              custom_sorting_t << " compared to " << default_sorting_t << "\n";
    std::cout << "\n" << std::endl;
}

TEST(PERFORMANCE_TESTS, TIMSORTING_RATIONALS) {
    const size_t DATA_SIZE = 10000;
    Deque<Rational> custom_deque;
    std::deque<Rational> default_deque;
    for (size_t i = 0; i < DATA_SIZE; ++i) {
        custom_deque.push_front(Rational(rand(), rand() + 1));
        default_deque.push_front(*custom_deque.begin());
    }
    ASSERT_EQ(custom_deque.size(), DATA_SIZE);

    SET_TIMER;
    ASSERT_NO_THROW(timSort(custom_deque.begin(), custom_deque.end()));
    STOP_TIMER;
    double custom_sorting_t = MEASURED_T;
    SET_TIMER;
    timSort(default_deque.begin(), default_deque.end());
    STOP_TIMER;

    double default_sorting_t = MEASURED_T;
    for (size_t i = 0; i < DATA_SIZE - 1; ++i) {
        ASSERT_TRUE(custom_deque[i] <= custom_deque[i + 1]);
    }
    std::cout << "TimSorting " << DATA_SIZE << " random rational numbers: " <<
              custom_sorting_t << " compared to " << default_sorting_t << "\n";

    SET_TIMER;
    ASSERT_NO_THROW(timSort(custom_deque.rbegin(), custom_deque.rend()));
    STOP_TIMER;
    custom_sorting_t = MEASURED_T;

    SET_TIMER;
    timSort(default_deque.rbegin(), default_deque.rend());
    STOP_TIMER;
    default_sorting_t = MEASURED_T;

    for (size_t i = 0; i < DATA_SIZE - 1; ++i) {
        ASSERT_TRUE(custom_deque[i] >= custom_deque[i + 1]);
    }

    EXPECT_LE(custom_sorting_t, default_sorting_t);

    std::cout << "TimSorting " << DATA_SIZE << " random rational numbers in reversed order: " <<
              custom_sorting_t << " compared to " << default_sorting_t << "\n";
    std::cout << "\n" << std::endl;
}

int main(int argc, char **argv) {
    srand(time(NULL));
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}