/*
 * 文件名   : inverted_index_manager.h 
 * 功能描述 : 倒排索引管理器，完成数据读取和倒排建立以及提供定向接口
 */

#pragma once

#include "common.h"
#include "bitset.h"
#include "line_segment.h"
#include "direct_value_index.h"

typedef struct {
    bool is_whitelist;
    std::vector<LineSegment> line_seg_vec;
}LineSegmentDirect;

class InvertedIndexManager
{
    public:
        InvertedIndexManager() {

        }

        ~InvertedIndexManager() {

        }

        typedef std::unordered_map<int, std::unordered_map<std::string, std::string>>  RequestDirectDataMp;
    public:
        // 建立倒排入口函数
        bool build(const DirectFieldInfoMap& f_mp, const DirectDataInfoVector& v_vec);

        // 请求倒排入口函数
        void request(const DirectRequestDataInfoMap& req_mp, DirectedIdSet& set, FilterReasonMap& filter_mp);

    private:
        bool generate_inverted_index();
        bool record_single_direct_value_info(bool is_case_sensitive, int uniq_id, const std::string& field_name, const DirectValueInfo& info);
        bool record_multi_direct_value_info(bool is_case_sensitive, int uniq_id, const std::string& field_name, const DirectValueInfo& info);
        bool record_line_segment_direct_value_info(bool is_case_sensitive, int uniq_id, const std::string& field_name, const DirectValueInfo& info, int direct_type);

        bool set_inverted_index();
        bool set_single_inverted_index();
        bool set_multi_inverted_index();
        bool set_line_segment_inverted_index();
        bool generate_all_direct_value_index(std::string direct_key, std::vector<int>& index_vec);

        bool parse_request_data_info(const DirectRequestDataInfoMap& req_mp, RequestDirectDataMp& req_data_mp);
        bool filter_data(const RequestDirectDataMp& req_data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp);
        bool single_direct_filter_data(const std::unordered_map<std::string, std::string>& data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp);
        bool multi_direct_filter_data(const std::unordered_map<std::string, std::string>& data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp);
        bool line_seg_direct_filter_data(const std::unordered_map<std::string, std::string>& data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp);

        bool response_data(const DynamicBitSet& bits, DirectedIdSet& id_set);
    private:
        inline bool is_need_inverted(int type) {
            return (DirectType_Inverted & type) == DirectType_Inverted;
        }

        inline bool is_single_direct(int type) {
            return (DirectType_SingleValue & type) == DirectType_SingleValue;
        }

        inline bool is_multi_direct(int type) {
            return (DirectType_MultiValue & type) == DirectType_MultiValue;
        }

        inline bool is_line_segment_direct(int type) {
            return (DirectType_LineSegment & type) == DirectType_LineSegment;
        }

        inline void record_filter_reason(const std::string& field, const DynamicBitSet& bits, FilterReasonMap& filter_mp) {
            for (DynamicBitSet::size_type uli = bits.first(); uli != DynamicBitSet::npos; uli = bits.next(uli)) {
                filter_mp.emplace(_src_direct_data_vec[uli].uniq_id, field);
            }

            return;
        }

    private:
        inline bool convert_str_to_number(const char* str, uint32_t& val) {
            char* prestrict_tail = NULL;
            val = strtol(str, &prestrict_tail, 0);
            return (*prestrict_tail == '\0');
        }


    private:
        DirectFieldInfoMap       _src_direct_field_mp;     // 存放定向字段名信息，以广告为例，存放的就是广告字段名
        DirectDataInfoVector     _src_direct_data_vec;     // 存放定向字段数据信息，以广告为例，存放的就是广告定向内容

    private:
        DirectValueIndex         _direct_value_index;      // 记录定向值索引、即为定向值分配整型索引，以广告为例，存放的就是广告定向内容与索引映射
        std::unordered_map<int, std::unordered_map<int, int>> _single_data_direct_mp;                // 记录单值数据设置的定向配置
        std::unordered_map<int, std::unordered_map<int, int>> _multi_data_direct_mp;                 // 记录多值数据设置的定向配置
        std::unordered_map<int, std::unordered_map<int, LineSegmentDirect>> _line_seg_direct_mp;     // 记录线段数据设置的定向配置

    private:
        std::unordered_map<int, DynamicBitSet> _single_inverted_index;   // 记录单值定向倒排索引

        std::unordered_map<int, DynamicBitSet> _multi_inverted_index;                      // 记录多值定向倒排索引
        std::unordered_map<std::string, MultiDynamicBitSet> _field_multi_inverted_index;   // 针对定向字段的倒排索引

        std::unordered_map<int, LineSegmentInvertedIndex> _line_seg_inverted_index;        // 记录线段定向的倒排索引
        std::unordered_map<int, MultiDynamicBitSet> _field_line_seg_inverted_index;        // 针对线段定向字段的倒排索引
};





