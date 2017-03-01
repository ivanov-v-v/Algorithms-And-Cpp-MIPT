#ifndef TIMSORT_TIMSORT_H
#define TIMSORT_TIMSORT_H
#include <vector>
#include <utility>
#include <algorithm>    // swap, iter_swap, swap_ranges, reverse, copy, lower_bound
#include <iterator>     // iterator_traits
#include <type_traits>  // is_same
#include <cmath>        // fabs, sqrt

#include "timsort_merge.h"
#include "timsort_stack.h"
#include "timsort_params.h"

using std::vector;
using std::pair;
using std::iter_swap;
using std::swap_ranges;
using std::prev;
using std::next;
using std::iterator_traits;

namespace timsort {
//-------------------------------------------------------------------------------------
    template<class RAI, class Compare>
    vector<pair<RAI, uint32_t>> extractRuns (RAI first, RAI last, Compare isLess, const ITimSortParams &params) {
        uint32_t minRun = params.minRun(last - first);
        vector<pair<RAI, uint32_t>> runs;
        runs.reserve((last - first) / minRun + 2);

        RAI runFirst = first;
        while (last - runFirst > 1) {
            bool strictlyDescending = isLess(*next(runFirst), *runFirst);
            RAI runLast = next(runFirst);
            while (runLast + 1 != last && (strictlyDescending == isLess(*next(runLast), *runLast))) {
                ++runLast;
            }
            if (strictlyDescending) {
                std::reverse(runFirst, runLast);
            }
            bool incompleteRun = false;
            while (runLast != last && runLast - runFirst < minRun) {
                incompleteRun = true;
                ++runLast;
            }
            if (incompleteRun) {
                insertionSort(runFirst, runLast, isLess);
            }
            uint32_t runLen = runLast - runFirst;
            runs.push_back(std::make_pair(runFirst, runLen));
            runFirst = runLast;
        }
        if (runFirst != last) {
            while (runFirst != last) {
                ++runs.back().second;
                ++runFirst;
            }
            insertionSort(runs.back().first, last, isLess);
        }
        return runs;
    }

//-------------------------------------------------------------------------------------
    template<class RAI, class Compare>
    void runTimSort (RAI first, RAI last, Compare isLess, const ITimSortParams &params = *(new DefaultParams)) {
        static_assert(
                std::is_same<std::random_access_iterator_tag, typename iterator_traits<RAI>::iterator_category>::value,
                "Containers without random accessing are not supported\n"
         );
        if (last - first <= params.getMinMerge()) {
            insertionSort(first, last, isLess);
            return;
        }

        typedef pair<RAI, uint32_t> run;
        vector<run> runs = extractRuns(first, last, isLess, params);
        timsort::stack<run> s;
        for (int currRun = 0; currRun != runs.size(); ++currRun) {
            s.push(runs[currRun]);
            while (s.size() >= 3) {
                run Z = s.pop(), Y = s.pop(), X = s.pop();
                uint32_t lenZ = Z.second, lenY = Y.second, lenX = X.second;
                EWhatMerge actionType = params.whatMerge(lenX, lenY, lenZ);
                if (actionType == WM_NoMerge) {
                    s.push(X), s.push(Y), s.push(Z);
                    break;
                } else if (actionType == WM_MergeYZ) {
                    mergeRuns(Y, Z, isLess, params);
                    s.push(X), s.push(Y);
                } else if (actionType == WM_MergeXY) {
                    mergeRuns(X, Y, isLess, params);
                    s.push(X), s.push(Z);
                } else {
                    if (params.needMerge(lenY, lenZ)) {
                        mergeRuns(Y, Z, isLess, params);
                        s.push(X), s.push(Y);
                    }
                }
            }
            if (s.size() >= 2) {
                run Y = s.pop(), X = s.pop();
                uint32_t lenY = Y.second, lenX = X.second;
                if (params.needMerge(lenX, lenY)) {
                    mergeRuns(X, Y, isLess, params);
                    s.push(X);
                } else {
                    s.push(X), s.push(Y);
                }
            }
        }
        while (s.size() > 1) {
            run Y = s.pop(), X = s.pop();
            mergeRuns(X, Y, isLess, params);
            s.push(X);
        }
    }
}
//-------------------------------------------------------------------------------------
template <class RandomAccessIterator>
void timSort(RandomAccessIterator first, RandomAccessIterator last) {
    typedef typename iterator_traits<RandomAccessIterator>::value_type valueType;
    class Compare {
    public:
        Compare(){}
        bool operator()(const valueType& lhs, const valueType& rhs) const {
            return lhs < rhs;
        }
    };
    timsort::runTimSort(first, last, Compare());
}
template <class RandomAccessIterator, class Compare>
void timSort(RandomAccessIterator first, RandomAccessIterator last, Compare isLess) {
    timsort::runTimSort(first, last, isLess);
}
//-------------------------------------------------------------------------------------
#endif //TIMSORT_TIMSORT_H