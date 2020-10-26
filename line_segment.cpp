
#include "line_segment.h"

std::list<LineSegment>& merge_line_segment(std::list<LineSegment>& li, LineSegment line_seg)
{
    bool is_end_in_list = false;

    // 查找IP段中可能和目标数据存在交集的情况，即元素的end大于目标的begin的情况
    auto iter = std::lower_bound(li.begin(), li.end(), line_seg);
    for (; !is_end_in_list && iter != li.end(); ++iter)
    {
        LineSegment line_seg_tmp(line_seg);
        if (iter->end >= line_seg.end) {
            is_end_in_list = true;
            if(iter->begin > line_seg.end)  {
                li.insert(iter, line_seg);
                break;
            }
        } else {
            line_seg_tmp.end = iter->end;
            line_seg.begin = iter->end + 1;
        }

        if (*iter == line_seg_tmp) {
            continue;
        }

        LineSegment line_segs[3] = {
            LineSegment {std::min(line_seg_tmp.begin, iter->begin), std::max(line_seg_tmp.begin, iter->begin) - 1},
            LineSegment {std::max(line_seg_tmp.begin, iter->begin), line_seg_tmp.end},
            LineSegment {line_seg_tmp.end + 1, iter->end}};

        bool isvalid[2] = {line_segs[0].is_valid(), line_segs[2].is_valid()};
        if (isvalid[0]) {
            if(isvalid[1]) {
                li.insert(iter, line_segs[0]);
                li.insert(iter, line_segs[1]);
                iter->begin = line_seg_tmp.end + 1;
            } else {
                li.insert(iter, line_segs[0]);
                iter->begin = std::max(line_seg_tmp.begin, iter->begin);
            }
        } else {
            li.insert(iter, line_segs[1]);
            iter->begin = line_seg_tmp.end + 1;
        }
    }

    if (!is_end_in_list)
    {
        li.emplace_back(line_seg);
    }

    return li;
}

bool LineSegmentInvertedIndex::record_line_segment(const LineSegment& line_seg)
{
	  merge_line_segment(_line_seg_list, line_seg);
	  return true;
}

void LineSegmentInvertedIndex::build_index(int direct_type, std::function<void(int direct_type, LineSegment& seg)> build_func)
{
    for(auto& seg : _line_seg_list) {
        build_func(direct_type, seg);
    }
}

void LineSegmentInvertedIndex::copy_line_segment()
{
    std::copy(_line_seg_list.begin(), _line_seg_list.end(), std::back_inserter(_line_seg_vector));
}

const LineSegment* LineSegmentInvertedIndex::find_seg_val(uint32_t seg_val) const
{
    int32_t pos = get_position(_line_seg_vector, seg_val);
    if (pos != -1) {
        return &_line_seg_vector[pos];
    }

    return NULL;
}
