/*
 * 文件名   : direct_value_index.h 
 * 功能描述 : 为定向值生成索引，每种定向值都有唯一索引，索引查找快于是定向值查找
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class DirectValueIndex
{
    public:
        DirectValueIndex() : _last_index(0) {
        
        }

        ~DirectValueIndex() {

        }

    public:
        // 获取定向值索引
        inline int find(const std::string& direct_value) const {   
            auto iter = _value_index_mp.find(direct_value);
            return iter != _value_index_mp.end() ? iter->second : -1;
        }

        // 插入定向值
        inline int insert(const std::string& direct_value) {
            int32_t index = find(direct_value);
            if (index >= 0) return index;

            index = _last_index++;
            if (_last_index < 0) return -1;

            _value_vec.push_back(direct_value);
            _value_index_mp.emplace(direct_value, index);

            return index;
        }

        // 根据索引获取定向值
        inline std::string to_string(int index) const {
            return static_cast<size_t>(index) < _value_vec.size() ? _value_vec[index] : "";
        }

        // 获取定向值数量
        inline size_t count() const {
            return _value_index_mp.bucket_count();
        }

    private:
        int _last_index;		                                         // 当前最新的索引
        std::vector<std::string> _value_vec;                         // 定向值集合
        std::unordered_map<std::string, int> _value_index_mp;        // 定向值与索引的映射
};




