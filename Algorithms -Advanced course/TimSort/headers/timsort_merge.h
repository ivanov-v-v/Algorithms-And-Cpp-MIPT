#ifndef TIMSORT_TIMSORT_MERGE_H
#define TIMSORT_TIMSORT_MERGE_H
#include <vector>
#include <utility>
#include <algorithm>    // swap, iter_swap, swap_ranges, reverse, copy, lower_bound, get_temporary_buffer, return_temporary_buffer
#include <iterator>     // iterator_traits
#include <cmath>        // fabs, sqrt

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
    void insertionSort (RAI l, RAI r, Compare isLess) {
        for (RAI i = l; i != r; ++i) {
            RAI j = i;
            while (l != j && isLess(*j, *prev(j))) {
                iter_swap(prev(j), j);
                --j;
            }
        }
    }
//-------------------------------------------------------------------------------------
    template<class RAI, class Compare>
    uint32_t gallop (RAI first, RAI last, const typename iterator_traits<RAI>::value_type &entry, Compare isLess) {
        uint32_t r = 1;
        while (first + r < last && isLess(*(first + r), entry)) {
            r <<= 1;
        }
        if (first + r >= last) {
            r = last - first;
        }
        return std::lower_bound(first, first + r - 1, entry, isLess) - first;
    }

//-------------------------------------------------------------------------------------
    template<class RAI, class Compare>
    void mergeOnSwap (RAI first, RAI middle, RAI last, RAI bufferFirst, Compare isLess, const ITimSortParams &params) {
        swap_ranges(first, middle, bufferFirst);
        RAI bufferLast = bufferFirst + (middle - first);

        const uint32_t MIN_GALLOP = params.getGallop();
        uint32_t fromLeft = 0, fromRight = 0;
        while (bufferFirst != bufferLast && middle != last) {
            if (fromLeft > MIN_GALLOP) {
                uint32_t offset = gallop(bufferFirst, bufferLast, *middle, isLess);
                swap_ranges(first, first + offset, bufferFirst);
                first += offset;
                bufferFirst += offset;
                fromLeft = fromRight = 0;
            } else if (fromRight > MIN_GALLOP) {
                uint32_t offset = gallop(middle, last, *bufferFirst, isLess);
                swap_ranges(first, first + offset, middle);
                first += offset;
                middle += offset;
                fromRight = fromLeft = 0;
            } else {
                if (isLess(*middle, *bufferFirst)) {
                    iter_swap(first, middle);
                    ++middle;
                    ++fromRight, fromLeft = 0;
                } else {
                    iter_swap(first, bufferFirst);
                    ++bufferFirst;
                    ++fromLeft, fromRight = 0;
                }
                ++first;
            }
        }
        while (bufferFirst != bufferLast) {
            iter_swap(first, bufferFirst);
            ++bufferFirst;
            ++first;
        }
    }

//-------------------------------------------------------------------------------------
    template<class RAI, class Compare>
    void merge (RAI l1, RAI l2, RAI r2, typename iterator_traits<RAI>::value_type *lBuff, Compare isLess) {
        typedef typename iterator_traits<RAI>::value_type valueType;
        valueType *rBuff = lBuff + (l2 - l1);
        std::copy(l1, l2, lBuff);
        while (lBuff != rBuff && l2 != r2) {
            if (isLess(*l2, *lBuff)) {
                *l1 = *l2;
                ++l2;
            } else {
                *l1 = *lBuff;
                ++lBuff;
            }
            ++l1;
        }
        while (lBuff != rBuff) {
            *l1 = *lBuff;
            ++lBuff;
            ++l1;
        }
    }

//-------------------------------------------------------------------------------------
    template<class RAI, class Compare>
    void inplaceMerge(RAI first, RAI middle, RAI last, Compare isLess, const ITimSortParams& params) {
        uint32_t intervalLength = last - first;
        if (intervalLength <= params.getMinMerge()) {
            insertionSort(first, last, isLess);
            return;
        }
        uint32_t blockLength = static_cast<uint32_t>(sqrt(intervalLength));
        uint32_t remainder = intervalLength % blockLength;
        RAI buffer = first + blockLength * (intervalLength / blockLength - 1);

        RAI gapPosition = first + blockLength * ((middle - first) / blockLength);
        swap_ranges(gapPosition, gapPosition + blockLength, buffer);

        for (RAI blockFirst = first; blockFirst != buffer; blockFirst += blockLength) {
            RAI pivotBlockFirst = blockFirst;
            for (RAI nextBlockFirst = blockFirst + blockLength; buffer - nextBlockFirst >= blockLength; nextBlockFirst += blockLength) {
                if (isLess(*nextBlockFirst, *pivotBlockFirst) ||
                    (*nextBlockFirst == *pivotBlockFirst && isLess(*(nextBlockFirst + blockLength - 1), *(pivotBlockFirst + blockLength - 1)))) {
                    pivotBlockFirst = nextBlockFirst;
                }
            }
            swap_ranges(pivotBlockFirst, pivotBlockFirst + blockLength, blockFirst);
        }
        for (RAI blockFirst = first + blockLength; blockFirst != buffer; blockFirst += blockLength) {
            mergeOnSwap(blockFirst - blockLength, blockFirst, blockFirst + blockLength, buffer, isLess, params);
        }

        remainder += blockLength;
        insertionSort(buffer - remainder, last, isLess);
        blockLength = remainder;
        intervalLength -= blockLength;

        for (RAI blockFirst = first + intervalLength - blockLength; blockFirst - first >= blockLength; blockFirst -= blockLength) {
            mergeOnSwap(blockFirst - blockLength, blockFirst, blockFirst + blockLength, buffer, isLess, params);
        }

        insertionSort(first, first + 2 * blockLength, isLess);
        insertionSort(buffer, last, isLess);
    }
    //-------------------------------------------------------------------------------------
    template<class RAI, class Compare>
    void mergeRuns (pair<RAI, uint32_t> &r1, pair<RAI, uint32_t> &r2, Compare isLess, const ITimSortParams &params) {
        RAI first = r1.first, middle = r2.first, last = r2.first + r2.second;
        r1.second += r2.second;
        if (isLess(*prev(middle), *middle)) {
            return;
        }
        if (params.forceInplaceMerge()) {
            inplaceMerge(first, middle, last, isLess, params);
        } else {
            typedef typename iterator_traits<RAI>::value_type valueType;
            valueType *buffer = new valueType[middle - first];
            timsort::merge(first, middle, last, buffer, isLess);
            delete[] buffer;
        }
    }
//-------------------------------------------------------------------------------------
}
#endif //TIMSORT_TIMSORT_MERGE_H