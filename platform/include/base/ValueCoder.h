#pragma once

// 将大的数值类型用变长字节数据编码/解码
// 注意,这个不见得一定会压缩,有可能编码后数据还超过64位
// 事实上,最大的编码长度会达到10个字节

// 返回编码后的字节数,失败(Buffer不够大)则返回0
size_t EncodeU64(BYTE* pbyBuffer, size_t uBufferSize, uint64_t uValue);
size_t EncodeS64(BYTE* pbyBuffer, size_t uBufferSize, int64_t nValue);

// 如果成功,则返回编码字节数,失败(数据错误或者长度不完整)则返回0
size_t DecodeU64(uint64_t* puValue, const BYTE* pbyData, size_t uDataLen);
size_t DecodeS64(int64_t* pnValue, const BYTE* pbyData, size_t uDataLen);
