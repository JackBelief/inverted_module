#include <boost/algorithm/string.hpp>
#include "inverted_index_manager.h"
#include <iostream>
#include <algorithm>


bool InvertedIndexManager::parse_request_data_info(const DirectRequestDataInfoMap& req_mp, RequestDirectDataMp& req_data_mp) {
    for (const auto& element : _src_direct_field_mp) { 
        auto iter = req_mp.find(element.first);
        if (iter == req_mp.end()) {
            req_data_mp[element.second.direct_type][element.first] = "";
        } else {
            std::string request_data = iter->second;
            if (!element.second.is_case_sensitive) {
                transform(request_data.begin(), request_data.end(), request_data.begin(), ::tolower);
            }

            req_data_mp[element.second.direct_type][element.first] = request_data;
        }
    }

    return true;
}

bool InvertedIndexManager::filter_data(const RequestDirectDataMp& req_data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp) {
    bits.resize(_src_direct_data_vec.size(), 1);

    auto single_iter = req_data_mp.find(DirectType_SingleValue);
    if (single_iter != req_data_mp.end()) {
        single_direct_filter_data(single_iter->second, bits, filter_mp);
    }

    auto multi_iter = req_data_mp.find(DirectType_MultiValue);
    if (multi_iter != req_data_mp.end()) {
        multi_direct_filter_data(multi_iter->second, bits, filter_mp);
    }

    auto line_seg_iter = req_data_mp.find(DirectType_LineSegment);
    if (line_seg_iter != req_data_mp.end()) {
        line_seg_direct_filter_data(line_seg_iter->second, bits, filter_mp);
    }

    return true;
}

bool InvertedIndexManager::single_direct_filter_data(const std::unordered_map<std::string, std::string>& data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp)
{
    std::vector<std::string> field_vec;
    std::vector<std::vector<int>> index_vecs;
    for (auto const& element : data_mp) {
        std::vector<int> index_vec;
        generate_all_direct_value_index(element.first + DIRECTION_SPLIT + element.second, index_vec);
        index_vecs.emplace_back(index_vec);
        field_vec.emplace_back(element.first);
    }

    DynamicBitSet tmp_bits = bits;
    for (auto uli = 0; uli < index_vecs.size(); uli++) {
        for (auto const& index : index_vecs[uli]) {
            auto bit_iter = _single_inverted_index.find(index);
            if (_single_inverted_index.end() != bit_iter) {
                bits &= bit_iter->second;
                record_filter_reason(field_vec[uli], tmp_bits ^ bits, filter_mp);
                tmp_bits = bits;
                break;
            }
        }
    }

    return true;
}

bool InvertedIndexManager::multi_direct_filter_data(const std::unordered_map<std::string, std::string>& data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp)
{
    DynamicBitSet tmp_bits = bits;
    for (auto const& element : data_mp) {
        auto multi_field_iter = _field_multi_inverted_index.find(element.first);
        if (multi_field_iter == _field_multi_inverted_index.end()) {
            continue;
        }

        DynamicBitSet wb_set(multi_field_iter->second.w_b_bitset[0] | multi_field_iter->second.w_b_bitset[1]);
        if (wb_set.any()) {
            DynamicBitSet or_set;
            or_set.resize(_src_direct_data_vec.size());

            std::vector<std::string> value_pairs;
            boost::algorithm::split(value_pairs, element.second, boost::is_any_of(","));
            for (auto const& value : value_pairs) {
                int index = _direct_value_index.find(element.first + DIRECTION_SPLIT + value);
                if (index < 0) {
                    continue;
                }

                auto multi_set_iter = _multi_inverted_index.find(index);
                if (multi_set_iter != _multi_inverted_index.end()) {
                    or_set |= multi_set_iter->second;
                }
            }

            bits &= (wb_set.flip() | (multi_field_iter->second.w_b_bitset[0] & or_set) | (multi_field_iter->second.w_b_bitset[1] - or_set));
            record_filter_reason(element.first, tmp_bits ^ bits, filter_mp);
            tmp_bits = bits;
        }
    }

    return true;
}

bool InvertedIndexManager::line_seg_direct_filter_data(const std::unordered_map<std::string, std::string>& data_mp, DynamicBitSet& bits, FilterReasonMap& filter_mp)
{
    DynamicBitSet tmp_bits = bits;
    for (auto const& element : data_mp) {
        auto field_iter = _src_direct_field_mp.find(element.first);
        if (field_iter == _src_direct_field_mp.end()) {
            continue;
        }

        auto seg_iter = _line_seg_inverted_index.find(field_iter->second.direct_type);
        if (seg_iter == _line_seg_inverted_index.end()) {
            continue;
        }

        auto wb_iter = _field_line_seg_inverted_index.find(field_iter->second.direct_type);
        if (wb_iter == _field_line_seg_inverted_index.end()) {
            continue;
        }

        DynamicBitSet wb_set(wb_iter->second.w_b_bitset[0] | wb_iter->second.w_b_bitset[1]);
        if (wb_set.any()) {
            uint32_t seg_val;
            convert_str_to_number(element.second.c_str(), seg_val);

            DynamicBitSet or_set;
            or_set.resize(_src_direct_data_vec.size());

            const LineSegment* pLineSeg = seg_iter->second.find_seg_val(seg_val);
            if (pLineSeg != NULL) {
                or_set = pLineSeg->data;
            }

            bits &= (wb_set.flip() | (wb_iter->second.w_b_bitset[0] & or_set) | (wb_iter->second.w_b_bitset[1] - or_set));
            record_filter_reason(element.first, tmp_bits ^ bits, filter_mp);
            tmp_bits = bits;
        }
    }

    return true;
}

bool InvertedIndexManager::response_data(const DynamicBitSet& bits, DirectedIdSet& id_set)
{
    if (bits.size() != _src_direct_data_vec.size()) {
        return false;
    }

    for (DynamicBitSet::size_type uli = bits.first(); uli != DynamicBitSet::npos; uli = bits.next(uli)) {
        id_set.emplace(_src_direct_data_vec[uli].uniq_id);
    }

    return true;
}
