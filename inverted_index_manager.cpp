
#include <boost/algorithm/string.hpp>

#include "inverted_index_manager.h"

#include <iostream>


bool InvertedIndexManager::build(const DirectFieldInfoMap& f_mp, const DirectDataInfoVector& v_vec) {
    // step 1 : 记录定向信息
    _src_direct_field_mp.reserve(f_mp.size());
    for (auto const & element : f_mp) {
        _src_direct_field_mp.emplace(element.first, element.second);
    }

    _src_direct_data_vec.reserve(v_vec.size());
    for (auto const & element : v_vec) {
        _src_direct_data_vec.emplace_back(element);
    }

    // step 2 : 生成倒排
    return generate_inverted_index();
}

void InvertedIndexManager::request(const DirectRequestDataInfoMap& req_mp, DirectedIdSet& set, FilterReasonMap& filter_mp) {
    // step 1 : 解析请求参数获取请求定向数据对应的索引集合，并按照定向方式分类存储
    RequestDirectDataMp req_data_mp;
    parse_request_data_info(req_mp, req_data_mp);

    // step 2 : 根据倒排索引过滤数据
    DynamicBitSet bits;
    filter_data(req_data_mp, bits, filter_mp);

    // step 3 : 返回过滤后的数据
    response_data(bits, set);

    return;
}






