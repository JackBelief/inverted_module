/*
 * 文件名   : common.h 
 * 功能描述 : 共用结构定义
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

const std::string DIRECTION_DEFAULT("*");
const std::string DIRECTION_SPLIT("\001");

// 定向值配置枚举
enum DirectValueEnum {
    DirectValue_NoBid = 0,            // 定向配置不投
    DirectValue_Bid = 1,              // 定向配置投
    DirectValue_DefBid = 1000,        // 默认定向值定向配置投
};

// 定向方式枚举
enum DirectTypeEnum {
    DirectType_Inverted    = 1,       // 倒排定向(默认支持，用于后续扩展)
    DirectType_SingleValue = 2,       // 单值定向
    DirectType_MultiValue  = 4,       // 多值定向
    DirectType_LineSegment = 8,       // 线段定向
};

// 定向字段信息
struct DirectFieldInfo {
    bool is_case_sensitive;                 // 定向数据是否区分大小写
    int direct_type;                        // 定向类型（位图）
};

typedef std::unordered_map<std::string, DirectFieldInfo> DirectFieldInfoMap;     // key是定向字段名

// 定向字段数据信息
struct DirectValueInfo {
    bool is_whitelist;             // 是否是白名单
    std::string     value;         // 定向值内容
};

struct DirectDataInfo {
    int uniq_id;                                                  // 唯一标识
    std::unordered_map<std::string, DirectValueInfo> data_mp;     // 定向字段数据集合
};

typedef std::vector<DirectDataInfo> DirectDataInfoVector;

// 定向请求数据信息
typedef std::unordered_map<std::string, std::string> DirectRequestDataInfoMap;

typedef std::unordered_set<int> DirectedIdSet;
typedef std::unordered_map<int, std::string> FilterReasonMap;


