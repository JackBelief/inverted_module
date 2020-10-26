#include <boost/algorithm/string.hpp>
#include "inverted_index_manager.h"
#include "common.h"
#include <iostream>
#include <algorithm>

struct DirectValueInfo global_def_direct_value = {true, DIRECTION_DEFAULT};

bool InvertedIndexManager::generate_inverted_index() {
    // step 1 : 按照定向方式分别记录定向数据，要注意解决定向数据传空的问题
    for (auto const& data_element : _src_direct_data_vec) {
        for (auto const& field_element : _src_direct_field_mp) {
            auto iter = data_element.data_mp.find(field_element.first);
            if (iter == data_element.data_mp.end()) {
                // 定向维度不设置定向数据的情况下，默认投放
                if (is_single_direct(field_element.second.direct_type)) {
                    record_single_direct_value_info(field_element.second.is_case_sensitive, data_element.uniq_id, field_element.first, global_def_direct_value);
                } else if (is_multi_direct(field_element.second.direct_type)) {
                    record_multi_direct_value_info(field_element.second.is_case_sensitive, data_element.uniq_id, field_element.first, global_def_direct_value);
                } else if (is_line_segment_direct(field_element.second.direct_type)) {
                    record_line_segment_direct_value_info(field_element.second.is_case_sensitive, data_element.uniq_id, field_element.first, global_def_direct_value, field_element.second.direct_type);
                }
            } else {
                // 定向维度设置定向数据的情况下，解析定向数据
                if (is_single_direct(field_element.second.direct_type)) {
                    record_single_direct_value_info(field_element.second.is_case_sensitive, data_element.uniq_id, field_element.first, iter->second);
                } else if (is_multi_direct(field_element.second.direct_type)) {
                    record_multi_direct_value_info(field_element.second.is_case_sensitive, data_element.uniq_id, field_element.first, iter->second);
                } else if (is_line_segment_direct(field_element.second.direct_type)) {
                    record_line_segment_direct_value_info(field_element.second.is_case_sensitive, data_element.uniq_id, field_element.first, iter->second, field_element.second.direct_type);
                }
            }
        }
    }   

    // step 2 : 基于定向明细，设置倒排索引取值
    set_inverted_index();

    return true;
}

// 针对单值定向进倒排的记录处理
bool InvertedIndexManager::record_single_direct_value_info(bool is_case_sensitive, int uniq_id, const std::string& field_name, const DirectValueInfo& info) {
    bool hasdefault = false;                        // 默认标记，是否设置了默认定向方式
    std::vector<std::string> value_pairs;
		boost::algorithm::split(value_pairs, info.value, boost::is_any_of(","));    // 多个定向值之间使用","间隔

    int index;
    for (auto const & element : value_pairs) {
        if (element.empty()) {
            continue;
        }

        if (element == DIRECTION_DEFAULT) {
            hasdefault = true;
            index = _direct_value_index.insert(field_name + DIRECTION_SPLIT);
        } else {
            std::string element_value = element;
            if (!is_case_sensitive) {
                // 定向字段不区分大小写，则一律转成小写
                transform(element_value.begin(), element_value.end(), element_value.begin(), ::tolower);
            }

            index = _direct_value_index.insert(field_name + DIRECTION_SPLIT + element_value);
        }

        if (index < 0) {
            continue;
        }

        // 记录每个标识在各定向值上的定向配置结果
        _single_data_direct_mp[uniq_id][index] = info.is_whitelist ? DirectValue_Bid : DirectValue_NoBid; 

        // 为定向值索引分配bitset
        _single_inverted_index.emplace(index, DynamicBitSet());
    }

    // 默认定向处理
    if (hasdefault == false) {
        index = _direct_value_index.insert(field_name + DIRECTION_SPLIT);
        if (index < 0) {
            return true;
        }

        _single_data_direct_mp[uniq_id][index] = info.is_whitelist ? DirectValue_NoBid : DirectValue_DefBid;
        _single_inverted_index.emplace(index, DynamicBitSet());
    }
 
    return true;
}

// 针对多值定向倒排的记录处理
bool InvertedIndexManager::record_multi_direct_value_info(bool is_case_sensitive, int uniq_id, const std::string& field_name, const DirectValueInfo& info) {
    bool hasdefault = false;                        // 默认标记，是否设置了默认定向方式
    std::vector<std::string> value_pairs;
    boost::algorithm::split(value_pairs, info.value, boost::is_any_of(","));    // 多个定向值之间使用","间隔

    int index;
    for (auto const & element : value_pairs) {
        if (element.empty()) {
            continue;
        }

        if (element == DIRECTION_DEFAULT) {
            hasdefault = true;
            index = _direct_value_index.insert(field_name + DIRECTION_SPLIT);
        } else {
            std::string element_value = element;
            if (!is_case_sensitive) {
                // 定向字段不区分大小写，则一律转成小写
                transform(element_value.begin(), element_value.end(), element_value.begin(), ::tolower);
            }

            index = _direct_value_index.insert(field_name + DIRECTION_SPLIT + element_value);
        }

        if (index < 0) {
            continue;
        }

        // 记录每个标识在各定向值上的定向配置结果
        _multi_data_direct_mp[uniq_id][index] = info.is_whitelist ? DirectValue_Bid : DirectValue_NoBid;

        // 为定向值索引分配bitset，多值情况下，只记录非多值的
        if (element != DIRECTION_DEFAULT) {
            _multi_inverted_index.emplace(index, DynamicBitSet());
        }
    }

    // 默认定向处理
    if (hasdefault == false) {
        index = _direct_value_index.insert(field_name + DIRECTION_SPLIT);
        if (index < 0) {
            return true;
        }

        _multi_data_direct_mp[uniq_id][index] = info.is_whitelist ? DirectValue_NoBid : DirectValue_DefBid;
    }

    return true;
}

// 针对线段定向倒排的记录处理，线段定向数据配置样式： xxx1_xxx2,xxx3_xxx4,xxx5_xxx6
bool InvertedIndexManager::record_line_segment_direct_value_info(bool is_case_sensitive, int uniq_id, const std::string& field_name, const DirectValueInfo& info, int direct_type) {
    std::vector<std::string> value_pairs;
    boost::algorithm::split(value_pairs, info.value, boost::is_any_of(","));    // 多个定向值之间使用","间隔

    LineSegment seg;
    LineSegmentDirect ls;
    ls.is_whitelist = info.is_whitelist;
    for (auto const & element : value_pairs) {
        if (element.empty()) {
            continue;
        }

        if (element == DIRECTION_DEFAULT) {
            continue;
        } else {
            std::vector<std::string> line_seg_vec;
            boost::algorithm::split(line_seg_vec, element, boost::is_any_of("_"));
            if (line_seg_vec.size() != 2) {
                continue;
            }

            if (!convert_str_to_number(line_seg_vec[0].c_str(), seg.begin)) {
                continue;
            }

            if (!convert_str_to_number(line_seg_vec[1].c_str(), seg.end)) {
                continue;
            }

            if (seg.is_valid()) {
                ls.line_seg_vec.emplace_back(seg);
            }
        }
    }

    std::sort(ls.line_seg_vec.begin(), ls.line_seg_vec.end());
    _line_seg_direct_mp[uniq_id][direct_type] = ls;

    auto seg_iter = _line_seg_inverted_index.find(direct_type);
    if (seg_iter == _line_seg_inverted_index.end()) {
        _line_seg_inverted_index[direct_type] = LineSegmentInvertedIndex{};
    }

    auto field_iter = _field_line_seg_inverted_index.find(direct_type);
    if (field_iter == _field_line_seg_inverted_index.end()) {
        _field_line_seg_inverted_index[direct_type] = MultiDynamicBitSet{};
    }

    return true;
}

bool InvertedIndexManager::set_inverted_index() {
    set_single_inverted_index();
    set_multi_inverted_index();
    set_line_segment_inverted_index();

    return true;
}

bool InvertedIndexManager::set_single_inverted_index() {
    // 根据定向明细，设置倒排索引的取值
    size_t data_size = _src_direct_data_vec.size();
    for (auto& element : _single_inverted_index) {
        element.second.resize(data_size);

        std::vector<int> index_vec;
        generate_all_direct_value_index(_direct_value_index.to_string(element.first), index_vec);

        size_t pos = 0;
        for (auto const& data : _src_direct_data_vec) {
            for (auto const& index : index_vec) {
                auto iter = _single_data_direct_mp[data.uniq_id].find(index);
                if (_single_data_direct_mp[data.uniq_id].end() == iter) {
                    continue;
                }

                if (iter->second > DirectValue_NoBid) {
                    element.second.set(pos);
                }
                break;
            }
            pos++;
        }
    }

    return true;
}

bool InvertedIndexManager::set_multi_inverted_index() {
    // 根据定向明细，设置倒排索引的取值
    size_t data_size = _src_direct_data_vec.size();
    for (auto& element : _multi_inverted_index) {
        element.second.resize(data_size);

        size_t pos = 0;
        for (auto const& data : _src_direct_data_vec) {
            auto iter = _multi_data_direct_mp[data.uniq_id].find(element.first);
            if (_multi_data_direct_mp[data.uniq_id].end() != iter) {
                element.second.set(pos);
            }
            pos++;
        }
    }

    for (const auto& field_element : _src_direct_field_mp) {
        if (is_multi_direct(field_element.second.direct_type)) {
            int index = _direct_value_index.find(field_element.first + DIRECTION_SPLIT);
            if (index < 0) {
                continue;
            }

            if (_field_multi_inverted_index.find(field_element.first) == _field_multi_inverted_index.end()) {
                MultiDynamicBitSet multi_set;
                multi_set.w_b_bitset[0].resize(data_size);
                multi_set.w_b_bitset[1].resize(data_size);
                _field_multi_inverted_index[field_element.first] = multi_set;
            }

            size_t pos = 0;
            for (auto const& data : _src_direct_data_vec) {
                auto iter = _multi_data_direct_mp[data.uniq_id].find(index);
                if (_multi_data_direct_mp[data.uniq_id].end() != iter) {
                    if (iter->second > DirectValue_NoBid) {
                        _field_multi_inverted_index[field_element.first].w_b_bitset[1].set(pos);                    
                    } else {
                        _field_multi_inverted_index[field_element.first].w_b_bitset[0].set(pos);
                    }
                }
                pos++;
            }
        }
    }

    return true;
}

bool InvertedIndexManager::set_line_segment_inverted_index() {
    // 对每种线段类型建立黑白名单索引
    size_t data_size = _src_direct_data_vec.size();
    for (auto& element : _field_line_seg_inverted_index) {
        element.second.w_b_bitset[0].resize(data_size);
        element.second.w_b_bitset[1].resize(data_size);

        for (auto uli = 0; uli < _src_direct_data_vec.size(); uli++) {            
            auto iter = _line_seg_direct_mp[_src_direct_data_vec[uli].uniq_id].find(element.first);
            if (iter == _line_seg_direct_mp[_src_direct_data_vec[uli].uniq_id].end()) {
                continue;
            }

            if (iter->second.line_seg_vec.empty()) {
                continue;
            }

            for (auto const& seg : iter->second.line_seg_vec) {
                _line_seg_inverted_index[element.first].record_line_segment(seg);
            }

            if (iter->second.is_whitelist) {
                element.second.w_b_bitset[0].set(uli);
            } else {
                element.second.w_b_bitset[1].set(uli);
            }
        }
    }

    for (auto& seg_element : _line_seg_inverted_index) {
        seg_element.second.build_index(seg_element.first, [this](int direct_type, LineSegment& seg) {
            DynamicBitSet li;
            li.resize(this->_src_direct_data_vec.size());

		        DynamicBitSet seg_li(this->_field_line_seg_inverted_index[direct_type].w_b_bitset[0] | this->_field_line_seg_inverted_index[direct_type].w_b_bitset[1]);
            for (DynamicBitSet::size_type i = seg_li.first(); i != DynamicBitSet::npos; i = seg_li.next(i)) {
                auto iter = this->_line_seg_direct_mp[this->_src_direct_data_vec[i].uniq_id].find(direct_type);
                if (iter != this->_line_seg_direct_mp[this->_src_direct_data_vec[i].uniq_id].end()) {
                    if (-1 != get_position(iter->second.line_seg_vec, seg.begin)) {
                        li.set(i);
                    }
                }

		        }
		        seg.data = li;
	      });

        seg_element.second.copy_line_segment();
    }

    return true;
}

bool InvertedIndexManager::generate_all_direct_value_index(std::string direct_key, std::vector<int>& index_vec) {
    int index;
    if (direct_key.back() != DIRECTION_SPLIT[0]) {
        index = _direct_value_index.find(direct_key);
        if (index > -1) {
            index_vec.emplace_back(index);
        }
    }

    std::string::size_type pos = direct_key.find(DIRECTION_SPLIT[0]);
    if (pos != std::string::npos) {
        if (pos != direct_key.size() - 1) {
            direct_key.erase(pos + 1); 
        }
    }

    index = _direct_value_index.find(direct_key);
    if (index > -1) {
        index_vec.emplace_back(index);
    }

    return true;
}

