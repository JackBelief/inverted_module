#pragma once


#include <list>
#include <vector>

#include "bitset.h"

// 线段结构，用于线段倒排，如：年龄、IP，为了兼容性，采用多值的实现方式
struct LineSegment
{
    uint32_t begin = 0;
    uint32_t end = 0;
	  DynamicBitSet data;

	  LineSegment() : begin(0), end(0) {

	  }

    LineSegment(uint32_t b, uint32_t e) : begin(b), end(e) {

    }

    bool is_valid() {
        return begin <= end;
    }

    // 重载运算符
    bool operator < (uint32_t seg_val) const {
        return end < seg_val;
    }

    bool operator < (const LineSegment &o) const {
		    return end < o.begin;
	  }

    bool operator > (const LineSegment &o) const {
        return begin > o.end;
    }

    bool operator == (const LineSegment &o) const {
        return (begin == o.begin && end == o.end);
    }
};

class LineSegmentInvertedIndex{
    public:
        LineSegmentInvertedIndex() {

        }

        ~LineSegmentInvertedIndex() {

        }

    public:
        bool record_line_segment(const LineSegment& seg);
        const LineSegment* find_seg_val(uint32_t seg_val) const;
        void copy_line_segment();
        void build_index(int direct_type, std::function<void(int direct_type, LineSegment& seg)> build_func);

    public:
        inline const std::vector<LineSegment>& get_seg_vector() const {
            return _line_seg_vector;
        }

    private:
        std::list<LineSegment>   _line_seg_list;
        std::vector<LineSegment> _line_seg_vector;
};

inline int32_t get_position(const std::vector<LineSegment>& seg_vec, uint32_t seg_val)
{
    auto iter = std::lower_bound(seg_vec.begin(), seg_vec.end(), seg_val);
    if (iter != seg_vec.end()) {
        if (iter->begin <= seg_val && seg_val <= iter->end) {
			      return iter - seg_vec.begin();
		    }
	  }

	  return -1;
}


inline std::list<LineSegment>::const_iterator get_position(const std::list<LineSegment>& seg_list, uint32_t seg_val)
{
    auto const iter = lower_bound(seg_list.begin(), seg_list.end(), seg_val);
    if (iter != seg_list.end()) {
        if (iter->begin <= seg_val && seg_val <= iter->end) {
            return iter;
        }
    }

    return seg_list.end();
}


