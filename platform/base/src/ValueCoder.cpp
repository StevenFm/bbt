#include "Base.h"

size_t EncodeU64(BYTE* pbyBuffer, size_t uBufferSize, uint64_t uValue)
{
	size_t	uResult		= 0;
	BYTE*	pbyPos		= pbyBuffer;
	BYTE	byCode		= 0;

	do
	{
		XY_FAILED_JUMP(pbyPos - pbyBuffer < (int)uBufferSize);
		byCode = (BYTE)(uValue & 0x7F);
		uValue >>= 7;

		if (uValue > 0)
		{
			byCode |= 0x80;
		}

		*pbyPos++ = byCode;
	} while (uValue > 0);

	uResult = (size_t)(pbyPos - pbyBuffer);
Exit0:
	return uResult;
}

size_t EncodeS64(BYTE* pbyBuffer, size_t uBufferSize, int64_t nValue)
{
	uint64_t uValue = (uint64_t)nValue;

	// 将符号位挪到最低位
	if (nValue < 0)
	{
		uValue--;
		uValue = ~uValue;
		uValue <<= 1;
		uValue |= 0x1;
	}
	else
	{
		uValue <<= 1;
	}
	return EncodeU64(pbyBuffer, uBufferSize, uValue);
}

size_t DecodeU64(uint64_t* puValue, const BYTE* pbyData, size_t uDataLen)
{
    size_t			uResult         = 0;
	const BYTE*		pbyPos   		= pbyData;
	uint64_t		uCode			= 0;
	uint64_t		uValue			= 0;
	int				nMove			= 0;

	while (true)
	{
		XY_FAILED_JUMP(pbyPos - pbyData < (int)uDataLen);
		// 63的由来: 在编码时,把数据按照7bit一组一组的编码,最多10个组,也就是10个字节
		// 第1组无需移位,第2组右移7位,第3组......,第10组(其实只有1位有效)右移了63位;
		// 所以,在解码的时候,最多左移63位就结束了:)
		XY_FAILED_JUMP(nMove <= 63);
		uCode = (*pbyPos) & 0x7F;

		uValue |= (uCode << nMove);

		if (((*pbyPos++) & 0x80) == 0)
			break;

		nMove += 7;
	}

	*puValue = uValue;
	uResult = (size_t)(pbyPos - pbyData);
Exit0:
    return uResult;
}

size_t DecodeS64(int64_t* pnValue, const BYTE* pbyData, size_t uDataLen)
{
	size_t	 uCount		= 0;	
	uint64_t uValue		= 0;

	uCount = DecodeU64(&uValue, pbyData, uDataLen);
	if (uCount == 0)
		return 0;

	if (uValue & 0x1)
	{
		uValue >>= 1;
		uValue = ~uValue;
		uValue++;
	}
	else
	{
		uValue >>= 1;
	}

	*pnValue = (int64_t)uValue;

	return uCount;
}

