//
// Created by vvi on 02.11.16.
//

#ifndef TIMSORT_TIMSORT_PARAMS_H
#define TIMSORT_TIMSORT_PARAMS_H

#include <cstdint>

namespace timsort {
//-------------------------------------------------------------------------------------
    enum EWhatMerge {WM_NoMerge, WM_MergeXY, WM_MergeYZ};

    class ITimSortParams {
    public:
        virtual uint32_t minRun(uint32_t n) const = 0;
        virtual bool needMerge(uint32_t lenX, uint32_t lenY) const = 0;
        virtual EWhatMerge whatMerge(uint32_t lenX, uint32_t lenY, uint32_t lenZ) const = 0;
        virtual uint32_t getGallop() const = 0;
        virtual bool forceInplaceMerge() const {
            return true;
        }
        virtual uint32_t getMinMerge() const {
            return -1;
        };
    };

    class DefaultParams : public ITimSortParams {
    public:
        DefaultParams(){}
        uint32_t minRun(uint32_t n) const {
            uint32_t r = 0;
            while (n >= 64) {
                r |= n & 1;
                n >>= 1;
            }
            return n + r;
        }
        bool needMerge(uint32_t lenX, uint32_t lenY) const {
            return lenX <= lenY;
        }
        EWhatMerge whatMerge(uint32_t lenX, uint32_t lenY, uint32_t lenZ) const {
            if (lenX > lenY + lenZ && lenY > lenZ) {
                return WM_NoMerge;
            }
            if (lenX <= lenY + lenZ) {
                return lenX <= lenZ ? WM_MergeXY : WM_MergeYZ;
            }
        }
        uint32_t getGallop() const {
            return 7;
        }
        uint32_t getMinMerge() const {
            return 64;
        }
    };
    class DeprecateInplaceMergeParams : public ITimSortParams {
    public:
        DeprecateInplaceMergeParams(){}
        uint32_t minRun(uint32_t n) const {
            uint32_t r = 0;
            while (n >= 64) {
                r |= n & 1;
                n >>= 1;
            }
            return n + r;
        }
        bool needMerge(uint32_t lenX, uint32_t lenY) const {
            return lenX <= lenY;
        }
        EWhatMerge whatMerge(uint32_t lenX, uint32_t lenY, uint32_t lenZ) const {
            if (lenX > lenY + lenZ && lenY > lenZ) {
                return WM_NoMerge;
            }
            if (lenX <= lenY + lenZ) {
                return lenX <= lenZ ? WM_MergeXY : WM_MergeYZ;
            }
        }
        uint32_t getGallop() const {
            return 7;
        }
        bool forceInplaceMerge() const {
            return false;
        }
        uint32_t getMinMerge() const {
            return 64;
        }
    };
//-------------------------------------------------------------------------------------
}

#endif //TIMSORT_TIMSORT_PARAMS_H
