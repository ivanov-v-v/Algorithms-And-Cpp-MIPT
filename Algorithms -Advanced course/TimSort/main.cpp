#include <iostream>     // cin, cout
#include <vector>
#include <algorithm>    // stable_sort
#include <ctime>        // clock_t, time
#include <cstdlib>      // srand, rand
#include <string>
#include <cmath>        // fabs, sqrt
#include <iomanip>      // setprecision

#include "headers/timsort.h"
//-------------------------------------------------------------------------------------
using std::vector;
using std::cin;
using std::cout;
//-------------------------------------------------------------------------------------
class Benchmark {
private:
    clock_t start_, finish_;

    template<typename T>
    class Compare {
    public:
        Compare(){}
        bool operator()(const T& lhs, const T& rhs) const {
            return lhs < rhs;
        }
    };

    inline void setTimer() {
        start_ = clock();
    }
    inline void stopTimer() {
        finish_ = clock();
    }

    void displayResult(bool success) {
        if (!success) {
            throw std::logic_error("INCORRECT PROCEDURE");
        }
        cout << "PASSED! " << (finish_ - start_) / static_cast<double>(CLOCKS_PER_SEC) << "s ";
    }
    void displayDefaultSortResults() {
        cout << "COMPARED TO " << (finish_ - start_) / static_cast<double>(CLOCKS_PER_SEC) << "s\n";
    }

    void testRandomCStyleArray(const uint32_t dataSize, const timsort::ITimSortParams& params = timsort::DefaultParams()) {
        cout << "C-STYLE ARRAY, " << dataSize << " ELEMENTS: ";
        int *testArray = new int[dataSize];
        std::generate(testArray, testArray + dataSize, rand);
        int *copyArray = new int[dataSize];
        std::copy(testArray, testArray + dataSize, copyArray);

        setTimer();
        timsort::runTimSort(testArray, testArray + dataSize, Compare<int>(), params);
        stopTimer();
        displayResult(std::is_sorted(testArray, testArray + dataSize));

        setTimer();
        std::stable_sort(copyArray, copyArray + dataSize);
        stopTimer();
        displayDefaultSortResults();

        delete[] testArray;
        delete[] copyArray;
    }
    void testRandomVector(const uint32_t dataSize, const timsort::ITimSortParams& params = timsort::DefaultParams()) {
        cout << "RANDOM VECTOR, " << dataSize << " ELEMENTS: ";
        vector<int> testVector(dataSize);
        for (size_t i = 0; i < dataSize; ++i) {
            testVector[i] = rand() * rand();
        }
        vector<int> copyVector = testVector;

        setTimer();
        timsort::runTimSort(testVector.begin(), testVector.end(), Compare<int>(), params);
        stopTimer();
        displayResult(std::is_sorted(testVector.begin(), testVector.end()));

        setTimer();
        std::stable_sort(copyVector.begin(), copyVector.end());
        stopTimer();
        displayDefaultSortResults();
    }
    void testReversedRandomVector(const uint32_t dataSize, const timsort::ITimSortParams& params = timsort::DefaultParams()) {
        cout << "REVERSED RANDOM VECTOR, " << dataSize << " ELEMENTS: ";
        vector<int> testVector(dataSize);
        std::generate(testVector.begin(), testVector.end(), rand);
        std::sort(testVector.begin(), testVector.end(), std::greater<int>());
        vector<int> copyVector (testVector.begin(), testVector.end());

        setTimer();
        timsort::runTimSort(testVector.begin(), testVector.end(), Compare<int>(), params);
        stopTimer();
        displayResult(std::is_sorted(testVector.begin(), testVector.end()));

        setTimer();
        std::stable_sort(copyVector.begin(), copyVector.end());
        stopTimer();
        displayDefaultSortResults();
    }
    void testPartiallySortedVector(const uint32_t dataSize, const uint32_t blockLength, const timsort::ITimSortParams& params = timsort::DefaultParams()) {
        cout << "PARTIALLY SORTED DATA, " << dataSize <<  ": ";
        vector<int> testVector(dataSize);
        std::generate(testVector.begin(), testVector.end(), rand);
        vector<int>::iterator it = testVector.begin();
        for (; it + blockLength <= testVector.end(); it += blockLength) {
            std::stable_sort(it, it + blockLength);
        }
        std::stable_sort(it, testVector.end());
        vector<int> copyVector = testVector;

        setTimer();
        timsort::runTimSort(testVector.begin(), testVector.end(), Compare<int>(), params);
        stopTimer();
        displayResult(std::is_sorted(testVector.begin(), testVector.end()));

        setTimer();
        std::stable_sort(copyVector.begin(), copyVector.end());
        stopTimer();
        displayDefaultSortResults();
    }
    class Point3D {
    private:
        double x_, y_, z_;
        double EPS_ = 1e-7;
    public:
        Point3D(){}
        Point3D(double x, double y, double z): x_(x), y_(y), z_(z), EPS_(1e-7) {}
        bool operator==(const Point3D& another) const {
            return fabs(x_ - another.x_) < EPS_ &&
                   fabs(y_ - another.y_) < EPS_ &&
                   fabs(z_ - another.z_) < EPS_;
        }
        bool operator<(const Point3D& another) const {
            return x_ + EPS_ < another.x_ ||
                   (fabs(x_ - another.x_) < EPS_ && y_ + EPS_ < another.y_)  ||
                   (fabs(x_ - another.x_) < EPS_ && fabs(y_ - another.y_) < EPS_ && z_ + EPS_ < another.z_);
        }
    };
    void testPoints3D(const uint32_t dataSize, const timsort::ITimSortParams& params = timsort::DefaultParams()) {
        cout << "POINTS IN 3D, " << dataSize << ": ";
        vector<Point3D> testPoints(dataSize);
        for (Point3D& pt : testPoints) {
            pt = {rand(), rand(), rand()};
        }
        vector<Point3D> copyVector = testPoints;

        setTimer();
        timsort::runTimSort(testPoints.begin(), testPoints.end(), Compare<Point3D>(), params);
        stopTimer();
        displayResult(std::is_sorted(testPoints.begin(), testPoints.end()));

        setTimer();
        std::stable_sort(copyVector.begin(), copyVector.end());
        stopTimer();
        displayDefaultSortResults();
    }
    void testStrings(const uint32_t dataSize, const uint32_t stringSize, const timsort::ITimSortParams& params = timsort::DefaultParams()) {
        cout << "STRINGS, " << dataSize << " LINES OF LENGTH " << stringSize << " EACH: ";
        vector<std::string> testDictionary(dataSize);
        for (std::string& line : testDictionary) {
            for (size_t j = 0; j < stringSize; ++j) {
                line += rand() % ('z' - 'a' + 1) + 'a';
            }
        }
        vector<std::string> copyVector = testDictionary;

        setTimer();
        timsort::runTimSort(testDictionary.begin(), testDictionary.end(), Compare<std::string>(), params);
        stopTimer();
        displayResult(std::is_sorted(testDictionary.begin(), testDictionary.end()));

        setTimer();
        std::stable_sort(copyVector.begin(), copyVector.end());
        stopTimer();
        displayDefaultSortResults();
    }
public:
    Benchmark() {
        srand(time(0));
        start_ = finish_ = 0;
    }
    Benchmark(const timsort::ITimSortParams& params) {
        srand(time(0));
        start_ = finish_ = 0;
    }
    void performAnalysis(const timsort::ITimSortParams& params = timsort::DefaultParams()) {
        if (params.forceInplaceMerge()) {
            cout << "INPLACE MODE: POOR PERFORMANCE, NO MEMORY OVERHEAD\n";
        } else {
            cout << "MEMORY OVERHEAD, FAST MERGE OF RUNS\n";
        }
        cout << std::fixed << std::setprecision(7);
        testRandomCStyleArray(1000, params);
        testRandomVector(1, params);
        testRandomVector(10, params);
        testRandomVector(100, params);
        testRandomVector(1000, params);
        testRandomVector(10000, params);
        testRandomVector(100000, params);
        testRandomVector(1000000, params);
        testReversedRandomVector(1000000, params);
        testPartiallySortedVector(20, 10, params);
        testPartiallySortedVector(40, 10, params);
        testPartiallySortedVector(80, 8, params);
        testPartiallySortedVector(128, 1, params);
        testPartiallySortedVector(1024, 1, params);
        testPoints3D(1000, params);
        testStrings(1000, 1000, params);
    }
};
//-------------------------------------------------------------------------------------

int main () {
    Benchmark measurer;
    measurer.performAnalysis();
    return 0;
}