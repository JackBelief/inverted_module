#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <unordered_map>

#include "common.h"
#include "direct_value_index.h"
#include "inverted_index_manager.h"

using namespace std;

bool isDebug = false;
void init() {
    std::string InvertedTest = std::getenv("InvertedTest");
    if (InvertedTest == "true") {
        isDebug = true;
    }
}

int GetDirectType(const std::string& field){
    static std::unordered_set<std::string> multi_field_set = {"crtsize"};
    static std::unordered_set<std::string> line_segment_field_set = {"age"};

    auto iter = multi_field_set.find(field);
    if (multi_field_set.end() != iter) {
        return DirectType_MultiValue;
    }

    iter = line_segment_field_set.find(field);
    if (line_segment_field_set.end() != iter) {
        return DirectType_LineSegment;
    }

    return DirectType_SingleValue;
}


void split(const std::string& s, const std::string& delim, std::vector<std::string>& vec) {
    vec.clear();
    if (delim.empty()) {
        vec.emplace_back(s);
        return;
    }

    std::string::size_type last = 0;
    std::string::size_type index = s.find(delim, last);
    while (index != std::string::npos) {
        vec.emplace_back(s.substr(last, index - last));
        last = index + delim.length();
        index = s.find(delim, last);
    }

    vec.emplace_back(s.substr(last));
    return;
}

class AdInfoInvertedIndex {
    public:


    public:
        DirectFieldInfoMap df_mp;
        DirectDataInfoVector dd_vec;
        InvertedIndexManager* pInvertedManager;

        std::unordered_map<int, std::string> direct_field_index_mp;
};

// 解析文件首行——字段行
void ParseFieldLine(const std::string& line, AdInfoInvertedIndex* p) {
    std::vector<std::string> field_vec;
    split(line, "\t", field_vec);

    size_t index = 0;
    for (auto element : field_vec) {
        if (element[0] == '+') {
            std::string field = element.substr(1);
            std::transform(field.begin(), field.end(), field.begin(), ::tolower);

            DirectFieldInfo info;
            info.is_case_sensitive = true;
            info.direct_type = GetDirectType(field);            

            p->df_mp.emplace(field, info);
            p->direct_field_index_mp.emplace(index, field);
        }

        ++index;
    }

    if (isDebug) {
        std::cout << "=========================字段行========================" << std::endl;
        for (auto ele : p->df_mp) {
            std::cout << ele.first << "  " << ele.second.direct_type << std::endl;
        }
        std::cout << "==========================OVER=========================" << std::endl;
    }
}

void ParseDirectValue(const std::string& value, DirectValueInfo& info) {
    std::string::size_type pos = value.find(":");
    if (std::string::npos != pos) {
        info.value = value.substr(0, pos);

        pos = value.find(":1");
        info.is_whitelist = std::string::npos != pos;
    } else {
        info.is_whitelist = true;
        info.value = value;
    }
}

// 解析文其他行——数据行
void ParseDataLine(const std::string& line, AdInfoInvertedIndex* p) {
    std::vector<std::string> data_vec;
    split(line, "\t", data_vec);

    DirectDataInfo info;
    for (size_t index = 0; index < data_vec.size(); ++index) {
        if (index == 0) {
            info.uniq_id = atoi(data_vec[index].c_str());
            continue;
        }

        auto iter = p->direct_field_index_mp.find(index);
        if (iter == p->direct_field_index_mp.end()) {
            continue;
        }

        DirectValueInfo val;
        ParseDirectValue(data_vec[index], val);
        info.data_mp.emplace(iter->second, val);
    }

    p->dd_vec.emplace_back(info);

    if (isDebug) {
        std::cout << "=========================数据行========================" << std::endl;
        auto data = p->dd_vec.back();
        std::cout << "crtId=" << data.uniq_id << std::endl;
        for (auto ele : data.data_mp) {
            std::cout << ele.first << "  " << ele.second.is_whitelist << "  " << ele.second.value << std::endl;
        }
        std::cout << "==========================OVER=========================" << std::endl;
    }

    return;
}

AdInfoInvertedIndex* p = new(AdInfoInvertedIndex);
void LoadAdInfoFile() {
    std::string line;
    std::ifstream file("ad_info");

    int line_count = 0;
    while(getline(file, line)) {
        ++line_count;
        if (line_count == 1) {
            ParseFieldLine(line, p);
        } else {
            ParseDataLine(line, p);
        }
    }

    p->pInvertedManager = new(InvertedIndexManager);
    p->pInvertedManager->build(p->df_mp, p->dd_vec);
    return;
}

void RequestAd(const DirectRequestDataInfoMap& tmp, DirectedIdSet& set, FilterReasonMap& filter_mp) {
    p->pInvertedManager->request(tmp, set, filter_mp);
}

void LoadRequestDataInfo(DirectRequestDataInfoMap& mp) {
    std::string line;
    std::ifstream file("request_info");
    std::vector<std::string> req_vec;
    while(getline(file, line)) {
        split(line, "|", req_vec);

        if (req_vec.size() == 2) {
            mp.emplace(req_vec[0], req_vec[1]);
        }
    }

    if (isDebug) {
        std::cout << "========================请求数据=======================" << std::endl;
        for (auto ele : mp) {
            std::cout << ele.first << "  " << ele.second << std::endl;
        }
        std::cout << "==========================OVER=========================" << std::endl;
    }
}


int main() {
    init();
    LoadAdInfoFile();

    DirectRequestDataInfoMap tmp;
    LoadRequestDataInfo(tmp);

    DirectedIdSet set;
    FilterReasonMap filter_mp;
    RequestAd(tmp, set, filter_mp);

    std::cout << "定向明细:" << std::endl;
    for (auto const& element : set) {
        std::cout << element << "  ";
    }
    std::cout << std::endl;

    std::cout << "过滤明细:" << std::endl;
    for (auto const& element : filter_mp) {
        std::cout << element.first << " rsn=" << element.second << std::endl;
    }

    return 0;
}



#if 0
const std::string vendor = "vendor";
const std::string sex = "sex";
const std::string age = "age";

int main1() {
    DirectFieldInfo da,db,dc;
    da.is_case_sensitive = true;
    da.direct_type = DirectType_MultiValue;

    db.is_case_sensitive = true;
    db.direct_type = DirectType_SingleValue;

    dc.is_case_sensitive = true;
    dc.direct_type = DirectType_LineSegment;

    DirectFieldInfoMap df_mp;
    df_mp.emplace(vendor, da);
    df_mp.emplace(sex, db);
    df_mp.emplace(age, dc);

    DirectValueInfo c, d;
    c.is_whitelist = true;
    c.value = "oppo";

    d.is_whitelist = true;
    d.value = "M";

    DirectDataInfo e,f,g;
    e.uniq_id = 1001;
    e.data_mp.emplace(vendor, c);
    e.data_mp.emplace(sex, d);

    DirectValueInfo cc, dd;
    cc.is_whitelist = false;
    cc.value = "vivo";

    dd.is_whitelist = true;
    dd.value = "M";

    f.uniq_id = 1002;
    f.data_mp.emplace(vendor, cc);
    f.data_mp.emplace(sex, dd);

    DirectValueInfo gg;
    gg.is_whitelist = true;
    gg.value = "1_5,20_50,6_10";
////////////////    gg.value = "1_5,6_10,20_50";

    g.uniq_id = 1003;
    g.data_mp.emplace(age, gg);

    DirectDataInfoVector dd_vec;
    dd_vec.emplace_back(e);
    dd_vec.emplace_back(f);
    dd_vec.emplace_back(g);

    InvertedIndexManager* p = new(InvertedIndexManager); 
    p->build(df_mp, dd_vec);

    DirectRequestDataInfoMap  tmp;
    tmp.emplace(vendor, "oppo,vivo");
    tmp.emplace(sex, "M");
    tmp.emplace(age, "8");

    DirectedIdSet set;
    FilterReasonMap filter_mp;
    p->request(tmp, set, filter_mp);

    std::cout << "定向明细:" << std::endl;
    for (auto const& element : set) {
        std::cout << element << "  ";
    }
    std::cout << std::endl;

    std::cout << "过滤明细:" << std::endl;
    for (auto const& element : filter_mp) {
        std::cout << element.first << " rsn=" << element.second << std::endl;
    }

    return 0;
}

#endif





