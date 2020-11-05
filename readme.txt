工程文件说明：
1）common.h                          公共结构、枚举定义
2）inverted_index_manager.h          倒排索引管理器声明文件
3）bitset.h                          位图文件，对boost::dynamic_set的封装
4）direct_value_index.h              定向索引文件，为生成的定向条件分配索引
5）inverted_index_manager.cpp        倒排索引管理器实现文件
6）build.cpp                         倒排索引管理器模块实现文件，负责完成倒排建立
7）request.cpp                       倒排索引管理器模块实现文件，负责完成定向请求
8）line_segment.h                    线段定向声明文件
9）line_segment.cpp                  线段定向实现文件
10）self_test.cpp                    自测文件
11）ad_info                          供自测文件读取的广告文件，模拟广告配置
12）request_info                     供自测文件读取的请求文件，模拟请求数据

self_test.cpp文件中引入了环境变量，用于控制日志打印，请在执行bin文件前执行相关命令：
export InvertedTest=true
否则，从self_test.cpp文件中移除相关代码。

实现分享： https://studygolang.com/topics/12531