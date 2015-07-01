#pragma once

#include <string>
#include <vector>

struct XNetworkAdapterInfo
{
    std::string strName;
    std::string strIpAddr;
    std::string strEtherAddr;
};

typedef std::vector<XNetworkAdapterInfo> XNetworkAdapterTable;

BOOL QueryNetworkAdaptersInfo(XNetworkAdapterTable& adapterTable);

// 判断一个ip地址是否内网地址:
// [10.*.*.*]
// [172.16.*.*, 172.31.*.*]
// [192.168.*.*]
// 另外,非法地址也会返回false
BOOL IsInternalIP(const char szIP[]);

